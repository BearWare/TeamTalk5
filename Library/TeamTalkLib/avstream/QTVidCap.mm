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

#import <Cocoa/Cocoa.h>
#import <QTKit/QTKit.h>

#include "QTVidCap.h"
#include <myace/MyACE.h>

using namespace media;
using namespace vidcap;

@interface CaptureDelegate : NSObject
{
    CVImageBufferRef  mCurrentImageBuffer;
    VideoCaptureListener* m_listener;
    int m_frame_delay;
    ACE_UINT32 m_last_frame_tm;
}
- (void)captureOutput:(QTCaptureOutput *)captureOutput
  didOutputVideoFrame:(CVImageBufferRef)videoFrame
     withSampleBuffer:(QTSampleBuffer *)sampleBuffer
       fromConnection:(QTCaptureConnection *)connection;
@end

@implementation CaptureDelegate

- (id)init {
    [super init];
    m_listener = NULL;
    m_frame_delay = 1000;
    m_last_frame_tm = GETTIMESTAMP();
    return self;
}

- (id)initWithListener:(VideoCaptureListener*)listener framedelay:(NSInteger)frame_delay {
    
    self = [super init];
    if(self) {
        m_listener = listener;
        m_frame_delay = frame_delay;
        m_last_frame_tm = GETTIMESTAMP();
    }
    return self;
}

-(void)dealloc {
    [super dealloc];
}

- (void)captureOutput:(QTCaptureOutput *)captureOutput
  didOutputVideoFrame:(CVImageBufferRef)videoFrame
     withSampleBuffer:(QTSampleBuffer *)sampleBuffer
       fromConnection:(QTCaptureConnection *)connection {
    (void)captureOutput;
    (void)sampleBuffer;
    (void)connection;


    ACE_UINT32 now = GETTIMESTAMP();
    if(W32_LT(now, m_last_frame_tm + m_frame_delay))
        return;

    m_last_frame_tm = now;
	
    CVPixelBufferRef pixels;

    @synchronized (self) {
        //TODO: direct access
        pixels = CVBufferRetain(videoFrame);
    }
	
    CVPixelBufferLockBaseAddress(pixels, 0);
    void* baseaddress = CVPixelBufferGetBaseAddress(pixels);
    
    size_t width = CVPixelBufferGetWidth(pixels);
    size_t height = CVPixelBufferGetHeight(pixels);
    size_t rowBytes = CVPixelBufferGetBytesPerRow(pixels);

    //NSLog(@"Frame %dx%d row-bytes: %d at %u", width, height, rowBytes, now);
    media::VideoFrame video_frame;
    video_frame.frame = static_cast<const char*>(baseaddress);
    video_frame.frame_length = height * rowBytes;
    video_frame.width = width;
    video_frame.height = height;
    video_frame.top_down = true;
    m_listener->OnVideoCaptureCallback(video_frame, NULL);

    CVPixelBufferUnlockBaseAddress(pixels, 0);
    CVBufferRelease(pixels);
}

@end

struct MyVidCapSession : public QTVidCapSession
{
    QTCaptureDeviceInput* capdevin;
    QTCaptureSession* capsession;
    QTCaptureVideoPreviewOutput* capvidout;
    CaptureDelegate* capture;

    MyVidCapSession() : capdevin(NULL), capsession(NULL), capvidout(NULL),
        capture(NULL) { }
};


QTVidCap::QTVidCap()
{
}

void QueryVideoFormats(VidCapDevice& dev, QTCaptureSession* capsession)
{
    int resolutions[][2] = { { 1600, 1200 }, { 1280, 960 }, { 800, 600 },
                             { 768, 576 }, { 640, 480 }, { 400, 300 },
                             { 320, 240 }, {160,120} };

    NSDictionary *pixelBufferOptions;

    for(int i=0;i<sizeof(resolutions)/sizeof(resolutions[0]);i++) 
    {
        QTCaptureVideoPreviewOutput* capvidout = [[QTCaptureVideoPreviewOutput alloc] init];

        double width = resolutions[i][0];
        double height = resolutions[i][1];
        pixelBufferOptions = [NSDictionary dictionaryWithObjectsAndKeys:
                                           [NSNumber numberWithDouble:width], (id)kCVPixelBufferWidthKey,
                                           [NSNumber numberWithDouble:height], (id)kCVPixelBufferHeightKey,
                                           [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,nil];
                                                         
        [capvidout setPixelBufferAttributes:pixelBufferOptions];
        
        NSError* error;
        BOOL success = [capsession addOutput:capvidout error:&error];
        if(success) {

            VideoFormat vidfmt;
            vidfmt.fourcc = FOURCC_RGB32;
            vidfmt.width = resolutions[i][0];
            vidfmt.height = resolutions[i][1];

            int fps[][2] = { {15, 1}, {30,1}, {10, 1}, {1,1} };

            for(int j=0;j<sizeof(fps)/sizeof(fps[0]);j++) {
                vidfmt.fps_numerator = fps[j][0];
                vidfmt.fps_denominator = fps[j][1];
                dev.vidcapformats.push_back(vidfmt);
            }
            
            [capsession removeOutput:capvidout];
        }
        else {
            NSLog(@"Failed to add output %dx%d:%d", (int)width, (int)height, FOURCC_RGB32);
        }

        [capvidout release];
    }

}

vidcap_devices_t QTVidCap::GetDevices()
{
    vidcap_devices_t ret_devs;

    BOOL success;
    NSError* error;
    NSArray* devs = [QTCaptureDevice inputDevicesWithMediaType:QTMediaTypeVideo];

    for(QTCaptureDevice* capdev in devs)
    {
        VidCapDevice dev;
        NSString* nsname = [capdev localizedDisplayName];
        dev.api = ACE_TEXT("QTKit");
        dev.devicename = [nsname UTF8String];
        dev.deviceid = [[capdev uniqueID] UTF8String];

        BOOL success = [capdev open:&error];
        if(!success)
        {
            NSLog(@"QTVidCap: Failed to open dev: %@", [error localizedDescription]);
            continue;
        }

        QTCaptureDeviceInput* capdevin = [[QTCaptureDeviceInput alloc] initWithDevice:capdev];
        QTCaptureSession* capsession = [[QTCaptureSession alloc] init];

        success = [capsession addInput:capdevin error:&error];

        if(!success) {
            NSLog(@"QTVidCap: Failed to add input to session during query: %@", [error localizedDescription]);
        }
        else {
            QueryVideoFormats(dev, capsession);
        }
        
        [capsession release];
        [capdevin release];
        [capdev close];

        ret_devs.push_back(dev);
    }

    return ret_devs;
}

bool QTVidCap::StartVideoCapture(const ACE_TString& deviceid,
                                 const VideoFormat& vidfmt,
                                 VideoCaptureListener* listener)
{
    NSString* str = [[[NSString alloc] initWithUTF8String:deviceid.c_str()] autorelease];

    QTCaptureDevice* capdev = [QTCaptureDevice deviceWithUniqueID:str];
    
    if(capdev == nil)
        return false;

    NSError* error;
    BOOL success = [capdev open:&error];
    if(!success)
    {
        NSLog(@"QTVidCap: Failed to open dev: %@", [error localizedDescription]);
        return false;
    }

    QTCaptureDeviceInput* capdevin = [[QTCaptureDeviceInput alloc] initWithDevice:capdev];
    
    QTCaptureSession* capsession = [[QTCaptureSession alloc] init];
    
    success = [capsession addInput:capdevin error:&error];
    if(!success) {
        NSLog(@"QTVidCap: Failed to add input to session: %@", [error localizedDescription]);
        [capsession release];
        return false;
    }
    QTCaptureVideoPreviewOutput* capvidout = [[QTCaptureVideoPreviewOutput alloc] init];
    //QTCaptureDecompressedVideoOutput * capvidout = [[QTCaptureDecompressedVideoOutput alloc] init];

    double fps = (double)vidfmt.fps_numerator / (double)vidfmt.fps_denominator;
    if(fps < 1.0)
        fps = 1.0;

    NSInteger frame_delay = 1000 / fps;
    CaptureDelegate* capture;
    capture = [[CaptureDelegate alloc] initWithListener:listener framedelay:frame_delay];

    [capvidout setDelegate:capture ];

    NSDictionary *pixelBufferOptions ;
    pixelBufferOptions = [NSDictionary dictionaryWithObjectsAndKeys:
                                           [NSNumber numberWithDouble:1.0*vidfmt.width], (id)kCVPixelBufferWidthKey,
                                       [NSNumber numberWithDouble:1.0*vidfmt.height], (id)kCVPixelBufferHeightKey,
                                       [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,
                                       [NSNumber numberWithUnsignedInt:vidfmt.fps_numerator], @"fps_numerator",
                                       [NSNumber numberWithUnsignedInt:vidfmt.fps_denominator], @"fps_denominator",
                                       nil];

    [capvidout setPixelBufferAttributes:pixelBufferOptions];

    success = [capsession addOutput:capvidout error:&error];
    if(!success) {
        NSLog(@"QTVidCap: Failed to add output to session %@", error);
        [capture release];
        [capvidout release];
        [capsession release];
        return false;
    }

    wguard_t g(m_mutex);

    MyVidCapSession* newsession;
    ACE_NEW_NORETURN(newsession, MyVidCapSession());
    m_mListeners[listener] = vidcap_session_t(newsession);

    newsession->capdevin = capdevin;
    newsession->capsession = capsession;
    newsession->capvidout = capvidout;
    newsession->capture = capture;
    
    g.release();

    [capsession startRunning];

    //NSLog(@"Started capturing using device id: %@", str);

    return true;
}

bool QTVidCap::StopVideoCapture(VideoCaptureListener* listener)
{
    wguard_t g(m_mutex);

    vidcaplisteners_t::iterator ii = m_mListeners.find(listener);
    if(ii == m_mListeners.end())
        return false;

    MyVidCapSession& session = *static_cast<MyVidCapSession*>(ii->second.get());
    
    [session.capsession stopRunning];
    [[session.capdevin device] close];
    
    [session.capsession release];
    [session.capdevin release];
    [session.capvidout release];
    [session.capture release];

    m_mListeners.erase(listener);

    return true;
}

bool QTVidCap::GetVideoCaptureFormat(vidcap::VideoCaptureListener* listener,
                                     media::VideoFormat& vidfmt)
{
    wguard_t g(m_mutex);

    vidcaplisteners_t::iterator ii = m_mListeners.find(listener);
    if(ii == m_mListeners.end())
        return false;

    MyVidCapSession& session = *static_cast<MyVidCapSession*>(ii->second.get());

    NSDictionary *pixelBufferOptions = [session.capvidout pixelBufferAttributes];

    NSString* width = [pixelBufferOptions objectForKey:(id)kCVPixelBufferWidthKey];
    NSString* height = [pixelBufferOptions objectForKey:(id)kCVPixelBufferHeightKey];
    NSString* imgfmt = [pixelBufferOptions objectForKey:(id)kCVPixelBufferPixelFormatTypeKey];
    NSString* fps_numerator = [pixelBufferOptions objectForKey:@"fps_numerator"];
    NSString* fps_denominator = [pixelBufferOptions objectForKey:@"fps_denominator"];

    vidfmt.width = [width intValue];
    vidfmt.height = [height intValue];
    vidfmt.fps_numerator = [fps_numerator intValue];
    vidfmt.fps_denominator = [fps_denominator intValue];
    switch([imgfmt intValue])
    {
    case kCVPixelFormatType_32BGRA :
        vidfmt.fourcc = FOURCC_RGB32;
        break;
    default :
        vidfmt.fourcc = FOURCC_NONE;
        break;
    }
    return true;
}

void CocoaEventLoop()
{
    [[NSRunLoop currentRunLoop] run];
}

