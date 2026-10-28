# PROJECT KNOWLEDGE BASE — leopard-mountd

挂载守护进程：读 `/system/config/fstab`，逐行挂载文件系统。

## fstab 格式
    # <device> <mountpoint> [fstype]
    /dev/sda2 /apps vfat

## 构建
    python3 gen_ninja.py && ninja build   # → out/leopard-mountd.elf
    ninja pack

- 静态 ELF，依赖 `../leopard-sdk`。
