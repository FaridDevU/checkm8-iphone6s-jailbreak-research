# checkm8 / iPhone 6s (A9) notes

Notes from jailbreaking an iPhone 6s (iPhone8,1, A9/S8000) on iOS 15.8.8,
using checkm8 (CVE-2019-8900) via palera1n v3.0.0-beta.2, on a Linux host.
Read [`DISCLAIMER.md`](DISCLAIMER.md) before anything else in here.

Device: SRTG `iBoot-2234.0.0.3.3`, CPID 0x8000, BDID 04, 16 GB, running
on a Kali Linux host over an AMD USB4/xHCI setup, which turned out to
matter for a couple of the issues documented below.

## What's here

- [`docs/bugs/01-palera1n-B-flag-noop.md`](docs/bugs/01-palera1n-B-flag-noop.md)
  covers `-B` not creating a fakefs on 16 GB devices.
- [`docs/bugs/02-jbinit-fbi-bind-panic.md`](docs/bugs/02-jbinit-fbi-bind-panic.md)
  covers the partial fakefs panic on first boot (`jbinit DIED!`).
- [`docs/walkthrough.md`](docs/walkthrough.md) has the steps that got this
  specific device jailbroken and past the activation screen.
- [`docs/research-log.md`](docs/research-log.md) is the raw working log
  from the whole session, dead ends included.
- `tools/pongo_console.c` and `tools/pongo_console2.c` are small libusb
  clients I wrote to talk to the PongoOS console directly, since I needed
  to read the KPF output and push flags without going through the
  palera1n CLI.

Both bugs above are about palera1n/jbinit rather than checkm8 itself,
since checkm8 is a hardware bootrom bug (CVE-2019-8900) that is well
known and cannot be patched by anyone. The bugs documented here live in
the jailbreak tooling that gets built on top of that exploit instead.

## License

[`LICENSE`](LICENSE) (GPL-3.0) covers my own writing and the two `.c`
files in `tools/`. I am not redistributing palera1n, checkra1n, or gaster
here; those remain separate projects with their own licenses.
