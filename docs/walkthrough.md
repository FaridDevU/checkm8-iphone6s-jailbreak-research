# How I got this device jailbroken and past activation

Read [`../DISCLAIMER.md`](../DISCLAIMER.md) first. This is what worked on
one specific iPhone 6s on iOS 15.8.8. A Linux box with
libirecovery/libimobiledevice, palera1n v3.0.0-beta.2, and the tools in
`../tools/` built is needed for all of this.

## 1. Pwn it

DFU mode, then:

```
palera1n -P -d -e "wdt=-1"
```

That runs checkm8 and parks the device in PongoOS without booting
further. Avoid palera1n v2.4 for this step on an AMD USB4/xHCI host; it
stalls intermittently on this hardware, while v3 beta.2 did not.

## 2. Get a fakefs out of it

`-B` is broken on 16 GB devices (see `bugs/01-palera1n-B-flag-noop.md`),
so instead of the CLI flag, push the flags through the PongoOS console:

```
pongo_console2 "palera1n_flags 0x80000015"
```

Then either wait for it to auto-boot or send `bootx` through the same
console. If it works, `setup_fakefs()` copies the system, skipping 14
big directories to fit in the free space, snapshots it as `orig-fs`,
writes the device to NVRAM, and reboots into recovery.

## 3. Make the empty mount points

The partial fakefs skips those 14 directories but does not leave empty
folders where they would go, so the first boot panics trying to
bind-mount onto them (`bugs/02-jbinit-fbi-bind-panic.md`). SSH into the
ramdisk (root/alpine) and create these, empty, inside the fakefs:

```
System/Library/Frameworks
System/Library/AccessibilityBundles
System/Library/Assistant
System/Library/Audio
System/Library/Fonts
System/Library/Health
System/Library/LinguisticData
System/Library/OnBoardingBundles
System/Library/Photos
System/Library/PreferenceBundles
System/Library/PreinstalledAssetsV2
System/Library/PrivateFrameworks
System/Library/Caches
usr/standalone/update
```

## 4. Deal with activation

Swap in a patched `usr/libexec/mobileactivationd` (the community
"setupapp" patch), re-signed with the stock binary's entitlements so it
still runs with AMFI off:

```
ldid -e /mnt1/usr/libexec/mobileactivationd.stock > entitlements.plist
ldid -S entitlements.plist /mnt1/usr/libexec/mobileactivationd.patched
```

Then clear out the local setup state so Setup Assistant actually runs
instead of jumping straight to the lock screen:

```
rm -f /mnt1/private/var/mobile/Library/Preferences/com.apple.purplebuddy.plist
rm -rf /mnt1/private/var/mobile/Library/.activation_records
```

The path for activation records moves around between iOS versions, so
it is worth checking what is actually present before running this.

## 5. Set the NVRAM and boot

```
nvram p1-fakefs-rootdev=disk0s1s8
nvram allow-root-hash-mismatch=1
nvram auto-boot=true
palera1n -f
```

The kernel boots the real root, `mountroot()` swaps `/` for the fakefs,
and `prepare_rootfs()` binds the real directories over the empty mount
points from step 3. Userland comes up through `prelaunchd()`, and Setup
Assistant runs for real this time instead of getting stuck on the old
owner's iCloud login.

## Notes

- This is semi-tethered. Every full reboot needs `palera1n -f` from DFU
  again, since the device does not boot into the patched state on its
  own.
- Avoid updating iOS, erasing content/settings, or logging into iCloud
  on this device. Any of those puts it back on the sealed system volume
  and restores the original lock.
- Entitlements in step 4 have to come from the exact same iOS version's
  stock binary. Mismatched entitlements fail codesign checks even with
  AMFI relaxed, since AMFI being off bypasses trust checks, not
  entitlement validation.
