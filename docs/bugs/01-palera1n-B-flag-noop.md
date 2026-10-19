# `-B` does not create a fakefs on 16 GB devices

iPhone 6s (A9/S8000, iBoot-2234.0.0.3.3), iOS 15.8.8, 16 GB. Kali Linux
host, AMD USB4/xHCI. palera1n v3.0.0-beta.2, official build.

16 GB devices are supposed to use `-B` (`--setup-partial-fakefs`) since
there is not enough free space for a full one. I ran `palera1n -Bf` and
the exploit runs fine, PongoOS boots, everything looks normal, but no
fakefs volume ever actually gets created. No error, no log line about
it, it just silently does not happen.

Went digging in `src/main.cpp`. The `-B` case only sets
`palerain_option_setup_partial_root` (bit `1<<4`). On the device the
flags end up as `0x80000011`, and bit `0x4` (`setup_rootful`) never gets
set, which turns out to matter a lot, because `prelaunchd.c` gates the
entire fakefs creation step on that exact bit:

```c
if (pinfo_p->flags & palerain_option_setup_rootful) {
    return setup_fakefs(payload_options, pinfo_p);
}
```

No `0x4`, no call to `setup_fakefs()`, nothing happens.

So what about `-c`, which does set `0x4`? It hits the space check inside
`setup_fakefs()` itself and panics, since a 16 GB device only has about
5 GB free:

```c
if ((attrbuf.spaceused + MINIMUM_EXTRA_SPACE) > (rootfs_st.f_bavail * rootfs_st.f_bsize)) {
    _panic("Not enough space! ...");
}
```

Net effect: there's currently no CLI combination on this build that
creates a partial fakefs on a 16 GB device.

Workaround that worked for me: stage without booting, then push the
combined flags straight through the PongoOS console instead of relying
on the CLI flag.

```
palera1n -P -d -e "wdt=-1"
```
then
```
palera1n_flags 0x80000015
bootx
```

`0x80000015` is rootful | setup_rootful | setup_partial_root | cli. With
both bits set, `setup_fakefs()` runs, takes the partial path, skips the
14 big dirs, creates the snapshot, and reboots into recovery like it is
supposed to. Worked first try, no issues since.

Probably just needs `-B` to also set `setup_rootful`. I didn't look into
what the intended fix is on your end, just posting what got it working
here in case it is useful.
