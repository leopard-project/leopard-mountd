# leopard-mountd

挂载守护进程：读 `/system/config/fstab`，逐行挂载文件系统。

## fstab 格式
```
# <device> <mountpoint> [fstype]
/dev/sata0p1 /apps fatfs
/dev/sata0p2 /users fatfs
```

fstype 缺省为 `fatfs`。**不要写 `vfat`**：内核 `get_filesystem()` 按名字精确 `strcmp`，
FAT 注册名就是 `fatfs`，写 `vfat` 会 -EINVAL 挂不上。

挂载点目录不存在会先 mkdir；mount 走内核 `SYS_MOUNT(165)` 原语。

安装器（leopard-installer）装完后会生成与三分区实测一致的 fstab；本包的
`data/fstab` 只是注释示例。

## 构建
```
python3 gen_ninja.py && ninja
```
