/*
 * Copyright (c) 2005-2025, BearWare.dk
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
package dk.bearware.utils;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.DnsResolver;
import android.net.LinkProperties;
import android.net.Network;
import android.os.Build;
import android.os.CancellationSignal;
import android.util.Log;

import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.IDN;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.security.SecureRandom;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

public final class DnsUtils {
    private static final String TAG = "bearware";
    private static final Executor DNS_EXECUTOR = Executors.newSingleThreadExecutor();
    private static final int DNS_TIMEOUT_MSEC = 1200;
    private static final SecureRandom DNS_RNG = new SecureRandom();
    private static final int DNS_PORT = 53;
    private static final int DNS_UDP_TIMEOUT_MSEC = 1500;
    private static final int DNS_UDP_MAX_PACKET = 1500;
    private static final int DNS_UDP_TOTAL_TIMEOUT_MSEC = 2500;
    private static final long DNS_CACHE_TTL_MSEC = 60_000;
    private static final long DNS_RESOLVER_COOLDOWN_MSEC = 60_000;

    private static final class CachedDns {
        final long expiresAt;
        final String[] orderedIps;

        CachedDns(long expiresAt, String[] orderedIps) {
            this.expiresAt = expiresAt;
            this.orderedIps = orderedIps;
        }
    }

    private static final Map<String, CachedDns> dnsCache = new ConcurrentHashMap<>();
    private static final Map<String, Long> resolverCooldownUntil = new ConcurrentHashMap<>();

    private DnsUtils() {
    }

    public static String[] resolveHostCandidatesForConnect(Context context, String host) {
        if (host == null)
            return new String[0];
        String trimmed = host.trim();
        if (trimmed.isEmpty())
            return new String[0];

        String normalized = stripBracketedIpv6(trimmed);
        if (isLikelyIpLiteral(normalized))
            return new String[] { normalized };

        long now = System.currentTimeMillis();
        CachedDns cached = dnsCache.get(normalized);
        if (cached != null && cached.orderedIps != null && cached.expiresAt > now && cached.orderedIps.length > 0) {
            return cached.orderedIps;
        }

        // Prefer resolving in Java/Android (tied to active Network) to avoid
        // platform resolver quirks in native getaddrinfo on some devices.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q && context != null) {
            try {
                ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
                Network network = (cm != null) ? cm.getActiveNetwork() : null;
                if (network != null) {
                    LinkProperties lp = cm.getLinkProperties(network);
                    Long cooldownUntil = resolverCooldownUntil.get(normalized);
                    if (cooldownUntil != null && cooldownUntil > now) {
                        // Skip resolver for a while if it previously timed out on this network.
                        InetAddress[] udpAddrs = resolveViaUdpDnsBlocking(normalized,
                                lp != null ? lp.getDnsServers() : null, lp);
                        String[] ips = toOrderedIps(udpAddrs, lp);
                        if (ips.length > 0) {
                            dnsCache.put(normalized, new CachedDns(now + DNS_CACHE_TTL_MSEC, ips));
                            Log.i(TAG, "DNS resolved (udp53 cached) " + normalized + " -> " + ips[0]);
                            return ips;
                        }
                        return new String[] { normalized };
                    }

                    CountDownLatch latch = new CountDownLatch(1);
                    final InetAddress[][] out = new InetAddress[1][];
                    final Throwable[] err = new Throwable[1];

                    CancellationSignal cancel = new CancellationSignal();
                    DnsResolver.getInstance().query(network, normalized, DnsResolver.FLAG_EMPTY, DNS_EXECUTOR,
                            cancel, new DnsResolver.Callback<java.util.List<InetAddress>>() {
                                @Override
                                public void onAnswer(java.util.List<InetAddress> answer, int rcode) {
                                    out[0] = (answer != null) ? answer.toArray(new InetAddress[0])
                                            : new InetAddress[0];
                                    latch.countDown();
                                }

                                @Override
                                public void onError(DnsResolver.DnsException e) {
                                    err[0] = e;
                                    latch.countDown();
                                }
                            });

                    boolean done = latch.await(DNS_TIMEOUT_MSEC, TimeUnit.MILLISECONDS);
                    if (!done) {
                        cancel.cancel();
                        resolverCooldownUntil.put(normalized, now + DNS_RESOLVER_COOLDOWN_MSEC);
                    } else if (err[0] != null) {
                        // Fall through to other resolvers.
                    } else {
                        InetAddress[] addrs = out[0];
                        if (addrs == null || addrs.length == 0) {
                            // Fall through to other resolvers.
                        } else {
                            // Respect system preference ordering from the resolver.
                            String[] ips = toOrderedIps(addrs, lp);
                            if (ips.length > 0) {
                                dnsCache.put(normalized, new CachedDns(now + DNS_CACHE_TTL_MSEC, ips));
                                return ips;
                            }
                        }
                    }

                    // UDP/53 fallback (unencrypted), for cases where Private DNS/resolver times out.
                    InetAddress[] udpAddrs = resolveViaUdpDnsBlocking(normalized,
                            lp != null ? lp.getDnsServers() : null, lp);
                    String[] ips = toOrderedIps(udpAddrs, lp);
                    if (ips.length > 0) {
                        dnsCache.put(normalized, new CachedDns(now + DNS_CACHE_TTL_MSEC, ips));
                        if (lp != null) {
                            String privateDns = "";
                            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
                                privateDns = " privateDnsActive=" + lp.isPrivateDnsActive() +
                                        " privateDnsServerName=" + lp.getPrivateDnsServerName();
                            }
                            Log.i(TAG, "DNS fallback (udp53) for " + normalized + ": dnsServers=" +
                                    lp.getDnsServers() + privateDns);
                        }
                        Log.i(TAG, "DNS resolved (udp53) " + normalized + " -> " + ips[0]);
                        return ips;
                    }
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                // Avoid log spam; resolution will fall back to hostname.
            } catch (RuntimeException e) {
                // Avoid log spam; resolution will fall back to hostname.
            }
        }

        // Fallback (older Android): InetAddress resolver.
        try {
            InetAddress[] addrs = InetAddress.getAllByName(normalized);
            if (addrs != null && addrs.length > 0) {
                String[] ips = toOrderedIps(addrs, null);
                if (ips.length > 0) {
                    dnsCache.put(normalized, new CachedDns(now + DNS_CACHE_TTL_MSEC, ips));
                    return ips;
                }
            }
        } catch (UnknownHostException e) {
            // Avoid log spam; resolution will fall back to hostname.
        }

        return new String[] { normalized };
    }

    private static String stripBracketedIpv6(String host) {
        if (host == null || host.isEmpty())
            return host;
        if (host.startsWith("[")) {
            int end = host.indexOf(']');
            if (end > 1)
                return host.substring(1, end);
        }
        return host;
    }

    private static boolean isLikelyIpLiteral(String host) {
        if (host == null || host.isEmpty())
            return false;
        // IPv6 literals contain ':' (usually more than one). Avoid treating
        // "host:port" as a literal.
        int firstColon = host.indexOf(':');
        if (firstColon >= 0) {
            int lastColon = host.lastIndexOf(':');
            if (firstColon != lastColon)
                return true;
            return false;
        }
        for (int i = 0; i < host.length(); i++) {
            char c = host.charAt(i);
            if (!(c == '.' || (c >= '0' && c <= '9')))
                return false;
        }
        return true;
    }

    private static InetAddress[] resolveViaUdpDnsBlocking(String hostname, List<InetAddress> dnsServers,
            LinkProperties lp) {
        if (dnsServers == null || dnsServers.isEmpty() || hostname == null || hostname.isEmpty())
            return null;

        // Avoid NetworkOnMainThreadException: run UDP DNS on a background executor.
        CountDownLatch latch = new CountDownLatch(1);
        final InetAddress[][] out = new InetAddress[1][];
        DNS_EXECUTOR.execute(() -> {
            try {
                out[0] = resolveViaUdpDns(hostname, dnsServers);
            } finally {
                latch.countDown();
            }
        });

        try {
            boolean done = latch.await(DNS_UDP_TOTAL_TIMEOUT_MSEC, TimeUnit.MILLISECONDS);
            return done ? out[0] : null;
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
            return null;
        }
    }

    private static InetAddress[] resolveViaUdpDns(String hostname, List<InetAddress> dnsServers) {
        if (dnsServers == null || dnsServers.isEmpty() || hostname == null || hostname.isEmpty())
            return null;

        InetAddress v6 = null;
        InetAddress v4 = null;
        for (InetAddress dns : dnsServers) {
            if (v6 == null)
                v6 = queryUdpDns(dns, hostname, 28 /* AAAA */);
            if (v4 == null)
                v4 = queryUdpDns(dns, hostname, 1 /* A */);
            if (v4 != null || v6 != null)
                break;
        }

        if (v6 != null && v4 != null)
            return new InetAddress[] { v6, v4 };
        if (v6 != null)
            return new InetAddress[] { v6 };
        if (v4 != null)
            return new InetAddress[] { v4 };
        return null;
    }

    private static String[] toOrderedIps(InetAddress[] addrs, LinkProperties lp) {
        if (addrs == null || addrs.length == 0)
            return new String[0];
        // Default: keep resolver order (system preference).
        // For the UDP fallback we provide IPv6 then IPv4; keep that order.
        String[] ips = new String[addrs.length];
        int n = 0;
        for (InetAddress a : addrs) {
            if (a == null)
                continue;
            String s = a.getHostAddress();
            if (s == null || s.isEmpty())
                continue;
            ips[n++] = s;
        }
        if (n == ips.length)
            return ips;
        String[] trimmed = new String[n];
        System.arraycopy(ips, 0, trimmed, 0, n);
        return trimmed;
    }

    private static InetAddress queryUdpDns(InetAddress dnsServer, String hostname, int qtype) {
        if (dnsServer == null || hostname == null || hostname.isEmpty())
            return null;

        int id = DNS_RNG.nextInt(0x10000);
        byte[] query = buildDnsQuery(id, hostname, qtype);
        if (query == null)
            return null;

        try (DatagramSocket socket = new DatagramSocket()) {
            socket.setSoTimeout(DNS_UDP_TIMEOUT_MSEC);
            InetSocketAddress dst = new InetSocketAddress(dnsServer, DNS_PORT);
            DatagramPacket out = new DatagramPacket(query, query.length, dst);
            socket.send(out);

            byte[] buf = new byte[DNS_UDP_MAX_PACKET];
            DatagramPacket in = new DatagramPacket(buf, buf.length);
            socket.receive(in);
            if (!dnsServer.equals(in.getAddress()) || in.getPort() != DNS_PORT)
                return null;

            return parseDnsResponseAddress(buf, in.getLength(), id, qtype);
        } catch (java.io.IOException e) {
            return null;
        }
    }

    private static byte[] buildDnsQuery(int id, String hostname, int qtype) {
        if (hostname.length() > 253)
            return null;
        if (hostname.endsWith("."))
            hostname = hostname.substring(0, hostname.length() - 1);
        if (hostname.isEmpty())
            return null;
        try {
            hostname = IDN.toASCII(hostname, IDN.USE_STD3_ASCII_RULES);
        } catch (IllegalArgumentException e) {
            return null;
        }

        ByteBuffer bb = ByteBuffer.allocate(512).order(ByteOrder.BIG_ENDIAN);
        bb.putShort((short) id);
        bb.putShort((short) 0x0100); // RD
        bb.putShort((short) 1); // QDCOUNT
        bb.putShort((short) 0); // ANCOUNT
        bb.putShort((short) 0); // NSCOUNT
        bb.putShort((short) 0); // ARCOUNT

        String[] labels = hostname.split("\\.");
        for (String label : labels) {
            if (label.isEmpty() || label.length() > 63)
                return null;
            bb.put((byte) label.length());
            bb.put(label.getBytes(StandardCharsets.US_ASCII));
        }
        bb.put((byte) 0); // end of QNAME
        bb.putShort((short) qtype);
        bb.putShort((short) 1); // IN

        byte[] out = new byte[bb.position()];
        bb.rewind();
        bb.get(out);
        return out;
    }

    private static InetAddress parseDnsResponseAddress(byte[] packet, int len, int expectedId, int expectedQtype) {
        if (packet == null || len < 12)
            return null;

        int id = readU16(packet, 0);
        if (id != expectedId)
            return null;

        int flags = readU16(packet, 2);
        boolean isResponse = (flags & 0x8000) != 0;
        if (!isResponse)
            return null;

        int rcode = flags & 0x000F;
        if (rcode != 0)
            return null;

        int qd = readU16(packet, 4);
        int an = readU16(packet, 6);

        int offset = 12;
        for (int i = 0; i < qd; i++) {
            offset = skipName(packet, len, offset);
            if (offset < 0 || offset + 4 > len)
                return null;
            offset += 4; // type + class
        }

        for (int i = 0; i < an; i++) {
            offset = skipName(packet, len, offset);
            if (offset < 0 || offset + 10 > len)
                return null;

            int type = readU16(packet, offset);
            int clazz = readU16(packet, offset + 2);
            int rdlen = readU16(packet, offset + 8);
            offset += 10;
            if (offset + rdlen > len)
                return null;

            if (clazz == 1 && type == expectedQtype) {
                try {
                    if (type == 1 && rdlen == 4) {
                        byte[] addr = new byte[4];
                        System.arraycopy(packet, offset, addr, 0, 4);
                        return InetAddress.getByAddress(addr);
                    }
                    if (type == 28 && rdlen == 16) {
                        byte[] addr = new byte[16];
                        System.arraycopy(packet, offset, addr, 0, 16);
                        return InetAddress.getByAddress(addr);
                    }
                } catch (UnknownHostException ignored) {
                    // ignore
                }
            }

            offset += rdlen;
        }

        return null;
    }

    private static int readU16(byte[] packet, int off) {
        return ((packet[off] & 0xFF) << 8) | (packet[off + 1] & 0xFF);
    }

    private static int skipName(byte[] packet, int len, int off) {
        int pos = off;
        while (pos < len) {
            int b = packet[pos] & 0xFF;
            if (b == 0) {
                return pos + 1;
            }
            if ((b & 0xC0) == 0xC0) {
                if (pos + 1 >= len)
                    return -1;
                return pos + 2;
            }
            int labellen = b;
            pos += 1;
            if (pos + labellen > len)
                return -1;
            pos += labellen;
        }
        return -1;
    }
}
