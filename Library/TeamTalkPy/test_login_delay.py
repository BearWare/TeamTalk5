"""Account login-delay protocol regressions against a disposable local server.

Run: python test_login_delay.py --server /absolute/path/to/tt5srv
The test starts its own server, bound ONLY to loopback, with temporary storage.
It never connects to an existing server. No audio devices are opened.
"""
import argparse
from contextlib import ExitStack
from pathlib import Path
import re
import secrets
import socket
import subprocess
import tempfile
import time
import unittest
import xml.etree.ElementTree as ET


class Client:
    def __init__(self, port):
        self.socket = socket.create_connection(("127.0.0.1", port), timeout=5)
        self.stream = self.socket.makefile("r", encoding="utf-8")
        self.counter = 0
        assert self.stream.readline().startswith("teamtalk")

    def close(self):
        self.stream.close()
        self.socket.close()

    def command(self, command):
        self.counter += 1
        self.socket.sendall(f"{command} id={self.counter}\r\n".encode())
        lines = []
        active = False
        while True:
            line = self.stream.readline().strip()
            if not line:
                raise RuntimeError("Server disconnected while waiting for command")
            if line == f"begin id={self.counter}":
                active = True
            elif active:
                if line == f"end id={self.counter}":
                    break
                lines.append(line)
        error = next((int(re.search(r"number=(\d+)", line)[1])
                      for line in lines if line.startswith("error ")), 0)
        return error, lines

    def login(self, username, password):
        return self.command(f'login username="{username}" password="{password}" '
                            'nickname="Login delay test" protocol="5.14" '
                            'clientname="login-delay-test" version="5.23"')[0]


class LoginDelayTests(unittest.TestCase):
    server_binary = None

    def setUp(self):
        self.stack = ExitStack()
        self.addCleanup(self.stack.close)
        self.directory = Path(self.stack.enter_context(tempfile.TemporaryDirectory(prefix="tt-login-delay-")))
        self.password = secrets.token_hex(24)
        with socket.socket() as probe:
            probe.bind(("127.0.0.1", 0))
            self.port = probe.getsockname()[1]
        self.config = self.directory / "server.xml"
        self.config.write_text(f'''<?xml version="1.0"?>
<teamtalk version="5.3"><general>
<server-name>Disposable login-delay test</server-name><max-users>100</max-users>
<bind-ip>127.0.0.1</bind-ip><tcpport>{self.port}</tcpport><udpport>{self.port}</udpport>
<user-timeout>60</user-timeout><login-delay-msec>0</login-delay-msec>
<auto-save>false</auto-save></general><users><user>
<username>admin</username><password>{self.password}</password><user-type>2</user-type>
<user-rights>0</user-rights>
<abuse-prevention><login-delay-msec>-1</login-delay-msec></abuse-prevention>
</user></users></teamtalk>''', encoding="utf-8")
        output = self.stack.enter_context((self.directory / "process.log").open("w"))
        process = subprocess.Popen([str(self.server_binary), "-nd", "-c", str(self.config),
                                    "-ip", "127.0.0.1", "-tcpport", str(self.port),
                                    "-udpport", str(self.port)], cwd=self.directory,
                                   stdout=output, stderr=subprocess.STDOUT,
                                   creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0))

        def stop():
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait(timeout=5)
        self.stack.callback(stop)
        for _ in range(100):
            if process.poll() is not None:
                self.fail((self.directory / "process.log").read_text())
            try:
                self.admin = self.client()
                break
            except ConnectionRefusedError:
                time.sleep(0.05)
        else:
            self.fail("Disposable server did not start")
        self.assertEqual(0, self.admin.login("admin", self.password))

    def client(self):
        client = Client(self.port)
        self.stack.callback(client.close)
        return client

    def success(self, command):
        error, lines = self.admin.command(command)
        self.assertEqual(0, error, lines)
        return lines

    def account(self, name, delay=0, usertype=1, legacy=False):
        values = "0,0" if legacy else f"0,0,{delay}"
        self.success(f'newaccount username="{name}" password="{self.password}" '
                     f'usertype={usertype} userrights=2 cmdflood=[{values}]')

    def attempt(self, name, expected=0):
        client = self.client()
        self.assertEqual(expected, client.login(name, self.password))
        return client

    def test_defaults_share_ip_including_admins(self):
        self.success("updateserver logindelay=3000")
        self.account("ordinary")
        self.account("other-admin", usertype=2)
        self.attempt("ordinary")
        self.attempt("other-admin", 2014)

    def test_exempt_accounts_do_not_refresh_other_accounts(self):
        self.success("updateserver logindelay=500")
        self.account("ordinary")
        self.account("exempt", -1)
        self.account("exempt-admin", -1, 2)
        self.attempt("exempt")
        self.attempt("ordinary")
        time.sleep(0.65)
        self.attempt("exempt-admin")
        self.attempt("ordinary")
        self.attempt("exempt")
        self.attempt("exempt-admin")

    def test_override_with_global_disabled_survives_cleanup(self):
        self.account("limited", 3000)
        self.attempt("limited")
        time.sleep(1.3)
        self.attempt("limited", 2014)
        time.sleep(3.2)
        self.attempt("limited")

    def test_short_and_long_overrides_are_independent(self):
        self.success("updateserver logindelay=10000")
        self.account("default")
        self.account("long", 5000)
        self.account("short", 200)
        self.attempt("default")
        self.attempt("long")
        self.attempt("short")
        time.sleep(0.3)
        self.attempt("short")
        self.attempt("long", 2014)
        self.attempt("default", 2014)

    def test_override_longer_than_global_survives_cleanup(self):
        self.success("updateserver logindelay=100")
        self.account("long", 3000)
        self.attempt("long")
        time.sleep(1.3)
        self.attempt("long", 2014)

    def test_live_delay_reduction(self):
        self.success("updateserver logindelay=10000")
        self.account("default")
        self.attempt("default")
        self.success("updateserver logindelay=200")
        time.sleep(0.3)
        self.attempt("default")
        self.account("override", 5000)
        self.attempt("override")
        self.account("override", 200)
        time.sleep(0.3)
        self.attempt("override")

    def test_same_socket_logout_and_login(self):
        self.account("limited", 5000)
        client = self.attempt("limited")
        self.assertEqual(0, client.command("logout")[0])
        self.assertEqual(2014, client.login("limited", self.password))
        self.account("limited", -1)
        self.assertEqual(0, client.login("limited", self.password))

    def test_legacy_edit_preserves_override_explicit_zero_resets(self):
        self.account("limited", 5000)
        self.account("limited", legacy=True)
        records = self.success("listaccounts index=0 count=100")
        self.assertTrue(any('username="limited"' in x and 'cmdflood=[0,0,5000]' in x for x in records), records)
        self.account("limited", 0)
        records = self.success("listaccounts index=0 count=100")
        self.assertTrue(any('username="limited"' in x and 'cmdflood=[0,0,0]' in x for x in records), records)

    def test_xml_persistence(self):
        for name, delay in (("default", 0), ("exempt", -1), ("limited", 5000)):
            self.account(name, delay)
        self.success("saveconfig")
        records = {x.findtext("username"): x for x in ET.parse(self.config).findall("./users/user")}
        for name, delay in (("default", 0), ("exempt", -1), ("limited", 5000)):
            self.assertEqual(str(delay), records[name].findtext("abuse-prevention/login-delay-msec"))

    def test_invalid_value_does_not_replace_account(self):
        self.account("limited", 5000)
        self.assertEqual(2002, self.admin.command(
            f'newaccount username="limited" password="{self.password}" usertype=1 cmdflood=[0,0,-2]')[0])
        records = self.success("listaccounts index=0 count=100")
        self.assertTrue(any('username="limited"' in x and 'cmdflood=[0,0,5000]' in x for x in records), records)

    def test_exemption_does_not_bypass_authentication(self):
        self.account("exempt", -1)
        client = self.client()
        self.assertEqual(2002, client.login("exempt", "wrong-password"))
        self.assertEqual(0, client.login("exempt", self.password))
        error, _ = client.command(f'newaccount username="intruder" password="x" usertype=2 cmdflood=[0,0,-1]')
        self.assertNotEqual(0, error)

    def test_malformed_delay_cannot_become_exemption(self):
        self.account("limited", 5000)
        for value in ("-2", "2147483648", "4294967295", "-4294967297", "no", "-", ""):
            with self.subTest(value=value):
                error, _ = self.admin.command(
                    f'newaccount username="limited" password="{self.password}" usertype=1 cmdflood=[0,0,{value}]')
                self.assertEqual(2002, error)
        records = self.success("listaccounts index=0 count=100")
        self.assertTrue(any('username="limited"' in x and 'cmdflood=[0,0,5000]' in x for x in records), records)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--server", required=True, type=Path)
    options, unittest_args = parser.parse_known_args()
    LoginDelayTests.server_binary = options.server.resolve(strict=True)
    unittest.main(argv=[__file__] + unittest_args, verbosity=2)
