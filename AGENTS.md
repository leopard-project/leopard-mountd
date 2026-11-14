# PROJECT KNOWLEDGE BASE — leopard-mountd

挂载守护进程：读 `/system/config/fstab`，逐行挂载文件系统。

## fstab 格式
    # <device> <mountpoint> [fstype]
    /dev/sata0p1 /apps fatfs
    /dev/sata0p2 /users fatfs

fstype 缺省 `fatfs`；写 `vfat` 会 -EINVAL（内核按名字精确 strcmp 匹配文件系统）。

## 构建
    python3 gen_ninja.py && ninja build   # → out/leopard-mountd.elf
    ninja pack

- 静态 ELF，依赖 `../XJ380-SDK`（`src/include` + `obj-tui`）。
