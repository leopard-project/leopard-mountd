# leopard-mountd

挂载守护进程：读 `/system/etc/leopard/fstab`，逐行挂载文件系统。

## fstab 格式
```
# <device> <mountpoint> [fstype]
/dev/sda2 /apps vfat
```

挂载点目录不存在会先 mkdir；mount 走内核 `SYS_MOUNT(165)` 原语。

## 构建
```
python3 gen_ninja.py && ninja
```
