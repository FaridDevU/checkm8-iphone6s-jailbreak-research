# partial fakefs dies on first boot - `jbinit DIED!`

Same device as the other report here: iPhone 6s, iOS 15.8.8, palera1n
v3.0.0-beta.2, fakedyld/jbinit as bundled with it (PongoOS-2.6.3-e98323f8).
KPF applied fine, rootful partial fakefs, RECOVERY role.

First boot after creating the fakefs, the device never comes up, and
the verbose console shows this right before it dies:

```
fakedyld: Skipping bind mount /usr/standalone/update because it does not exist
fakedyld: cannot bind /cores/fs/real/usr/standalone/update onto /usr/standalone/update, err=2
fakedyld: jbinit DIED!
```

err=2 is ENOENT, so it logs that it is skipping the mount and then
binding anyway and panicking when the target turns out to be missing.
That is the actual bug, and it is in `include/fakedyld/utils.h`, in the
`fbi()` macro:

```c
#define fbi(mnt, dir) do { \
    struct stat64 fbi_st; \
    if (stat64(mnt, &fbi_st)) { \
        LOG("Skipping bind mount %s because it does not exist", mnt); \
    } \
    int fbi_ret = mount("bindfs", mnt, MNT_RDONLY, dir); \
    if (fbi_ret != 0) panic("cannot bind %s onto %s, err=%d", dir, mnt, errno); \
    ...
```

The "skip" is just a log line, there is no actual early return, so it
falls through to `mount()` regardless and panics if that fails.

Why the target is missing in the first place: a partial fakefs skips
copying 14 big directories (the stuff under `System/Library/...` plus
`usr/standalone/update`) to save space, and `setup_fakefs()`'s copy
callback mkdirs on the source side when one of those is missing, never
inside the fakefs itself. So the bind target just never exists on first
boot.

Fix on my end was simple enough: mkdir the 14 directories inside the
fakefs before booting it (full list is in the walkthrough doc in this
repo). Once they are there as empty dirs, the binds go through fine and
boot proceeds normally.

Real fix is probably one of: make `fbi()` actually return on a failed
`stat64`, mkdir the target inside the macro before mounting, or have
`setup_fakefs()` create those empty dirs itself for a partial setup. Not
sure which matches the intended design here, so I will leave that to
whoever owns this code.
