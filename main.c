#include "xtuiapi.h"

/* leopard-mountd：读 /system/config/fstab，逐个挂载文件系统。
 * fstab 格式：<device> <mountpoint> [fstype]，# 开头为注释。
 * 挂载点目录不存在会先 mkdir；mount 走 SYS_MOUNT(165) 原语。
 */

#define FSTAB_PATH "/system/config/fstab"
#define BUF_CAP    (16 * 1024)

int main(int argc, char *argv[], char *envp[])
{
        (void)argc;
        (void)argv;
        (void)envp;

        char buf[BUF_CAP];
        memset(buf, 0, sizeof(buf));

        XFILE *fp = xapi_OpenFile((WSTR)FSTAB_PATH);
        if (fp == NULL) {
                xapi_Printf("[mountd]: no fstab at %s\n", FSTAB_PATH);
                return 0;
        }
        uint64_t len = fp->length;
        if (len >= sizeof(buf)) len = sizeof(buf) - 1;
        uint64_t got = xapi_ReadFile((WSTR)FSTAB_PATH, buf, len, 0);
        xapi_CloseFile(fp);
        buf[got] = '\0';

        int mounted = 0;
        char *line = buf;
        while (*line) {
                char *nl = strchr(line, '\n');
                if (nl) *nl = '\0';

                char *p = line;
                while (*p == ' ' || *p == '\t' || *p == '\r') p++;

                if (*p == '\0' || *p == '#') {
                        if (!nl) break;
                        line = nl + 1;
                        continue;
                }

                char *device = strtok(p, " \t\r");
                char *target = strtok(NULL, " \t\r");
                char *fstype = strtok(NULL, " \t\r");
                if (device == NULL || target == NULL) {
                        if (!nl) break;
                        line = nl + 1;
                        continue;
                }
                /* 内核 get_filesystem 精确匹配 fatfs */
                if (fstype == NULL) fstype = (char *)"fatfs";

                /* 确保挂载点目录存在 */
                xapi_Mkdir((WSTR)target);

                int64_t r = (int64_t)enter_syscall(SYS_MOUNT, (uint64_t)device, (uint64_t)target,
                                                   (uint64_t)fstype, 0, 0, 0);
                if (r == 0) {
                        xapi_Printf("[mountd]: %s -> %s (%s) ok\n", device, target, fstype);
                        mounted++;
                } else {
                        xapi_Printf("[mountd]: %s -> %s failed (ret=%d)\n", device, target, (int)r);
                }

                if (!nl) break;
                line = nl + 1;
        }

        xapi_Printf("[mountd]: %d filesystem(s) mounted\n", mounted);
        return 0;
}
