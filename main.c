#include "xtuiapi.h"
#include <stdarg.h>
#include <stdio.h>

/* leopard-mountd: reads /system/config/fstab and mounts filesystems one by one.
 * fstab format: <device> <mountpoint> [fstype]; lines starting with # are comments.
 * Missing mount point directories are created first; mount uses the SYS_MOUNT(165) primitive.
 */

#define FSTAB_PATH "/system/config/fstab"
#define BUF_CAP    (16 * 1024)

/* Create mount points level by level (xapi_Mkdir does not create parent directories). */
static void mkdir_p(const char *path)
{
        char buf[256];
        size_t len = strlen(path);

        if (len == 0 || len >= sizeof(buf)) return;
        memcpy(buf, path, len + 1);
        for (size_t i = 1; i <= len; i++) {
                if (buf[i] != '/' && buf[i] != '\0') continue;
                char saved = buf[i];

                buf[i] = '\0';
                xapi_Mkdir((WSTR)buf);
                buf[i] = saved;
        }
}

static void mountd_log(const char *format, ...)
{
        char    buffer[256];
        va_list args;
        int     length;

        va_start(args, format);
        length = vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        if (length > 0) {
                if ((size_t)length > sizeof(buffer) - 1) length = (int)sizeof(buffer) - 1;
                xapi_OutputSerial((WSTR)buffer);
        }
}

int main(int argc, char *argv[], char *envp[])
{
        (void)argc;
        (void)argv;
        (void)envp;

        char buf[BUF_CAP];
        memset(buf, 0, sizeof(buf));

        XFILE *fp = xapi_OpenFile((WSTR)FSTAB_PATH);
        if (fp == NULL) {
                mountd_log("[mountd]: no fstab at %s\n", FSTAB_PATH);
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

                char *comment = strchr(p, '#');
                if (comment != NULL) *comment = '\0';

                char *device = strtok(p, " \t\r");
                char *target = strtok(NULL, " \t\r");
                char *fstype = strtok(NULL, " \t\r");
                if (device == NULL || target == NULL) {
                        if (!nl) break;
                        line = nl + 1;
                        continue;
                }
                /* The third fstab column is passed to the kernel as-is (fatfs / ext4 / ...).
                 * If missing, still fall back to fatfs for the old format. */
                if (fstype == NULL) fstype = (char *)"fatfs";

                if (strcmp(target, "/") == 0) {
                        mountd_log("[mountd]: skip root entry (already mounted by kernel)\n");
                        if (!nl) break;
                        line = nl + 1;
                        continue;
                }

                /* Ensure the mount point directory exists (including parents) */
                mkdir_p(target);

                int64_t r = (int64_t)enter_syscall(SYS_MOUNT, (uint64_t)device, (uint64_t)target,
                                                   (uint64_t)fstype, 0, 0, 0);
                if (r == 0) {
                        mountd_log("[mountd]: %s -> %s (%s) ok\n", device, target, fstype);
                        mounted++;
                } else {
                        mountd_log("[mountd]: %s -> %s failed (ret=%d)\n", device, target, (int)r);
                }

                if (!nl) break;
                line = nl + 1;
        }

        mountd_log("[mountd]: %d filesystem(s) mounted\n", mounted);
        return 0;
}
