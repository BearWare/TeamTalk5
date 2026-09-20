# Account-specific login delays

The server-wide login delay remains the default. An administrator can override
it for an individual account in Qt's **User Accounts → Abuse Prevention** tab.
This is independent of the account's command-flood limit.

- **Use server login delay** (default): inherit the current server setting.
- Clear that checkbox and choose **Disabled** to exempt this account from the
  login delay. Password verification, bans, maximum connections and other
  flood protections still apply.
- Clear the checkbox and enter a positive interval in milliseconds to give
  this account a custom login delay. For example, `1000` means one second.

Administrators are not automatically exempt. Grant exemptions only to trusted
accounts that need them, such as a monitoring client.

## Accounting

Accounts that inherit the server setting still share its per-IP counter.
Each positive override has a separate account/IP counter, shared by all clients
using that account from the same IP. An override replaces the default; the
default is not an additional minimum. Multiple accounts with overrides have
independent allowances. Exempt accounts do not create or refresh these counters.
For web logins, the counter uses the authenticated username even when its settings
come from the shared `bearware` account.

The check happens after successful authentication and applies to both new TCP
connections and logout/login on an existing connection. An attempt made before
the interval expires returns `CMDERR_COMMAND_FLOOD` and renews the interval.
The authenticated account is passed to the delay check before `SetUserAccount()`
authorizes the connection. A rejected login does not acquire that account's rights.
Changing a delay affects the next attempt; it does not disconnect logged-in users.
Increasing a delay cannot retroactively recover already-expired counter history;
the first attempt after history has expired starts a new interval.
Expired entries are removed by the server's cleanup timer. Large configured
intervals keep entries for longer and should be used with care.

## SDK, protocol and configuration

`AbusePrevention.nLoginDelayMSec` uses `0` for inheritance, `-1` for disabled,
and positive values for milliseconds. This differs intentionally from
`ServerProperties.nLoginDelayMSec`, whose zero means disabled. Zero-initialized
account structures therefore preserve the existing default behavior.

On the wire, accounts use the existing `logindelay` property. `cmdflood` retains
its two command-flood values: `[commandCount, commandIntervalMSec]`. Account
parsing handles the login delay alongside the other account properties and
rejects malformed, overflowing, or below-`-1` values before updating an account.
The standalone server preserves an existing override when an older account
editor omits `logindelay`. Explicit `logindelay=0` resets the account to
inheritance. Older servers do not enforce account overrides.

XML stores `login-delay-msec` inside each user's `abuse-prevention` element.
Missing or invalid XML values inherit the server default. The server wizard
also offers the override for both ordinary and administrator accounts.
Applications hosting the server SDK manage their own account persistence.
The legacy-edit preservation described above is provided by the standalone
server, not the SDK's account-lookup stub. A legacy update passed to an SDK
host's account-creation callback still contains zero for the missing field;
the host must manage preservation itself rather than blindly replacing its
stored override. No account database is added to SDK-hosted applications.

The C structure layout has changed. Rebuild and distribute the native library,
headers and language bindings together; this is **not** a drop-in replacement
DLL for older compiled clients. Network compatibility is separate from SDK ABI
compatibility. The final SDK release/version assignment belongs to the maintainer.

## Regression tests

- Catch2: `ServerXML User Accounts Write/Read`, `Account login delay wire defaults`,
  `ServerXML login delay legacy and invalid values`, and
  `XML strict integer reading preserves legacy behavior`. `ServerNode clears all
  login delay counters on stop` checks in-process counter reset without opening sockets.
- JNI/JUnit: `testLoginDelay`, `testAccountLoginDelayExemption`, and
  `testAccountLoginDelayOverride`.
