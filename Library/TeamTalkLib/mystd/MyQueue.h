/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#if !defined(MYQUEUE_H)
#define MYQUEUE_H

#include <atomic>
#include <chrono>
#include <memory>
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <deque>
#include <vector>
#include <utility>

// Blocking MPMC queue, std-library replacement for ACE_Message_Queue
// (std::queue + std::mutex + std::condition_variable).
// Watermarks and reactor notification are not part of this class.
template <typename T>
class BlockingQueue
{
public:
    virtual ~BlockingQueue() = default;

    // Put (like ACE_Message_Queue::enqueue), returns true on success
    virtual bool Put(T item)
    {
        {
            std::scoped_lock<std::mutex> lk(m_mtx);
            if (m_closed)
                return false; // queue closed
            m_queue.push_back(std::move(item));
        }
        m_cv.notify_one();
        return true;
    }

    // Get with timeout (like ACE_Message_Queue::dequeue with ACE_Time_Value).
    // Returns false on timeout, or when closed and empty.
    virtual bool Get(T& out, std::chrono::milliseconds timeout = std::chrono::milliseconds::max())
    {
        std::unique_lock<std::mutex> lk(m_mtx);
        //wait without timeout when 'timeout' is max (wait_for with max() overflows the clock)
        if (timeout == std::chrono::milliseconds::max())
            m_cv.wait(lk, [this] { return !m_queue.empty() || m_closed; });
        else if (!m_cv.wait_for(lk, timeout, [this] { return !m_queue.empty() || m_closed; }))
            return false; // timed out
        if (m_queue.empty())
            return false; // closed and drained
        out = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }

    // PutFront (like ACE_Message_Queue::enqueue_head): insert at front
    virtual bool PutFront(T item)
    {
        {
            std::scoped_lock<std::mutex> lk(m_mtx);
            if (m_closed)
                return false; // queue closed
            m_queue.push_front(std::move(item));
        }
        m_cv.notify_one();
        return true;
    }

    // Flush (like ACE_Message_Queue::flush): remove all queued items.
    // The removed items are returned so derived classes can account
    // for them.
    virtual std::deque<T> Flush()
    {
        std::deque<T> removed;
        {
            std::scoped_lock<std::mutex> lk(m_mtx);
            removed.swap(m_queue);
        }
        return removed;
    }

    // Close (like ACE_Message_Queue::close): wakes up all waiters in Get()
    void Close()
    {
        {
            std::scoped_lock<std::mutex> lk(m_mtx);
            m_closed = true;
        }
        m_cv.notify_all();
    }

    // Activate (like ACE_Message_Queue::activate): (re)open a closed queue
    void Activate()
    {
        std::scoped_lock<std::mutex> lk(m_mtx);
        m_closed = false;
    }

    std::size_t Size() const
    {
        std::scoped_lock<std::mutex> lk(m_mtx);
        return m_queue.size();
    }

    bool IsClosed() const
    {
        std::scoped_lock<std::mutex> lk(m_mtx);
        return m_closed;
    }

    // Peek at front element without removing it; nullptr if empty.
    // Valid until the next Get/Close.
    const T* PeekFront() const
    {
        std::scoped_lock<std::mutex> lk(m_mtx);
        return m_queue.empty() ? nullptr : &m_queue.front();
    }

private:
    mutable std::mutex m_mtx;
    std::condition_variable m_cv;
    std::deque<T> m_queue;
    bool m_closed = false;
};

// Message block, similar to ACE_Message_Block (base/rd_ptr/wr_ptr semantics)
class MsgBlock
{
public:
    explicit MsgBlock(std::size_t capacity) : m_buf(capacity) { }
    MsgBlock(const void* src, std::size_t len) : m_buf(len) { Copy(src, len); }

    MsgBlock() = delete;
    MsgBlock(const MsgBlock&) = delete;

    char* Data() { return m_buf.data(); }
    const char* Data() const { return m_buf.data(); }
    char* Base() { return m_buf.data(); }
    char* RdPtr() { return m_buf.data() + m_rd; }
    // ACE semantics: advance read pointer by 'offset' bytes (rd_ptr(n) += n)
    void RdPtr(std::size_t offset) { m_rd = offset <= m_wr - m_rd ? m_rd + offset : m_wr; }
    // ACE semantics: set read pointer to absolute 'ptr' (rd_ptr(char*) = ptr)
    void RdPtr(char* ptr) { auto const off = static_cast<std::size_t>(ptr - m_buf.data()); m_rd = off <= m_wr ? off : m_wr; }
    char* WrPtr() { return m_buf.data() + m_wr; }
    // ACE semantics: advance write pointer by 'offset' bytes (wr_ptr(n) += n)
    void WrPtr(std::size_t offset) { m_wr = offset <= m_buf.size() - m_wr ? m_wr + offset : m_buf.size(); }
    char* End() { return m_buf.data() + m_buf.size(); }
    std::size_t Capacity() const { return m_buf.size(); }

    // ACE semantics: unread bytes (wr_ptr - rd_ptr)
    std::size_t Length() const { return m_wr - m_rd; }
    void Length(std::size_t len) { m_wr = m_rd + len <= m_buf.size() ? m_rd + len : m_buf.size(); }

    // ACE semantics (ACE_Message_Block::copy): copy 'len' bytes from 'src'
    // at the write pointer. The block never grows beyond its capacity.
    // Returns 0 on success, -1 if 'len' exceeds the free space after the
    // write pointer.
    int Copy(const void* src, std::size_t len)
    {
        if (len > m_buf.size() - m_wr)
            return -1;
        std::memcpy(m_buf.data() + m_wr, src, len);
        m_wr += len;
        return 0;
    }

private:
    std::vector<char> m_buf;
    std::size_t m_rd = 0; // read offset
    std::size_t m_wr = 0; // write offset
};

using msgblock_t = std::unique_ptr<MsgBlock>;

// BlockingQueue of MsgBlock* with a water mark on the maximum number of
// queued bytes (like ACE_Message_Queue::high_water_mark). Water mark 0
// means unlimited. Only uses the public interface of BlockingQueue.
class MsgQueue : public BlockingQueue<std::unique_ptr<MsgBlock>>
{
public:
    // Set maximum number of bytes allowed in queue (0 = unlimited)
    void SetWaterMark(std::size_t max_bytes) { m_water_mark.store(max_bytes); }
    std::size_t GetWaterMark() const { return m_water_mark.load(); }

    // Number of bytes currently queued (like ACE_Message_Queue::message_bytes,
    // i.e. sum of MsgBlock::Capacity())
    std::size_t BytesQueued() const { return m_bytes_queued.load(); }

    // Total length of the queued blocks, like ACE_Message_Queue::message_length()
    // ("sum of the message block lengths"): the sum of MsgBlock::Length() sampled
    // on each Put/PutFront/Get/Flush. MsgBlock::Length() is the block's unread size
    // in bytes (wr_ptr - rd_ptr), so this is a byte total of unread data — but not
    // of allocated buffers (that is BytesQueued(), like message_bytes(), which the
    // water mark checks) and not a block count (that is Size(), like message_count()).
    std::size_t LengthQueued() const { return m_length_queued.load(); }

    // Peek at head block without removing it (like ACE_Message_Queue::peek_dequeue_head);
    // nullptr if empty. Valid until the next Get/Close.
    MsgBlock* PeekHead() const
    {
        auto const* front = PeekFront();
        return front != nullptr ? front->get() : nullptr;
    }

    // Put fails (returns false) if the water mark would be exceeded.
    // The water mark is advisory, so the check-then-add race is benign.
    // Bytes are accounted by block capacity (ACE_Message_Block::size()),
    // like ACE_Message_Queue::message_bytes(), so blocks whose write
    // pointer was never advanced are still counted.
    bool Put(std::unique_ptr<MsgBlock> mb) override
    {
        auto const water_mark = m_water_mark.load();
        auto const len = mb->Capacity();
        if (water_mark != 0 && m_bytes_queued.load() + len > water_mark)
            return false;
        auto const ulen = mb->Length();
        if (!BlockingQueue::Put(std::move(mb)))
            return false;
        m_bytes_queued.fetch_add(len);
        m_length_queued.fetch_add(ulen);
        return true;
    }

    bool Get(std::unique_ptr<MsgBlock>& out, std::chrono::milliseconds timeout = std::chrono::milliseconds::max()) override
    {
        if (!BlockingQueue::Get(out, timeout))
            return false;
        m_bytes_queued.fetch_sub(out->Capacity());
        m_length_queued.fetch_sub(out->Length());
        return true;
    }

    // PutFront (like ACE_Message_Queue::enqueue_head)
    bool PutFront(std::unique_ptr<MsgBlock> mb) override
    {
        auto const len = mb->Capacity();
        auto const ulen = mb->Length();
        if (!BlockingQueue::PutFront(std::move(mb)))
            return false;
        m_bytes_queued.fetch_add(len);
        m_length_queued.fetch_add(ulen);
        return true;
    }

    // Flush (like ACE_Message_Queue::flush): remove all queued blocks
    std::deque<std::unique_ptr<MsgBlock>> Flush() override
    {
        auto removed = BlockingQueue::Flush();
        std::size_t bytes = 0, ulen = 0;
        for (auto const& mb : removed)
        {
            bytes += mb->Capacity();
            ulen += mb->Length();
        }
        m_bytes_queued.fetch_sub(bytes);
        m_length_queued.fetch_sub(ulen);
        return removed;
    }

private:
    std::atomic<std::size_t> m_water_mark{0};
    std::atomic<std::size_t> m_bytes_queued{0};
    std::atomic<std::size_t> m_length_queued{0};
};

#endif
