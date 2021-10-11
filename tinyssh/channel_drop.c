/*
20140129
Jan Mojzis
Public domain.
*/

#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include "dropuidgid.h"
#include "newenv.h"
#include "channel.h"

int channel_droppriv(char *user, char **shell) {

    struct passwd *pw;
    char *name;
    int do_droppriv = 1;

#ifdef __ANDROID__
    /*
    Yet another SEAndroid-causes-everything-to-break scenario.
    We have to give up on DROPPING privileges in order to have tinysshd work with non-root users.
    */
    if (geteuid() != 0) {
        do_droppriv = 0;
        pw = getpwuid(geteuid());
    }
    else {
        pw = getpwnam(user);
    }
#else
    pw = getpwnam(user);
#endif
    if (!pw) return 0;

    if (isatty(0)) {
        name = ttyname(0);
        if (!name) return 0;
        if (!newenv_env("SSH_TTY", name)) return 0;
        /* setowner */
        if (do_droppriv && chown(name, pw->pw_uid, pw->pw_gid) == -1) return 0;
        if (do_droppriv && chmod(name, 0600) == -1) return 0;
    }

    /* drop privileges */
    if (do_droppriv && !dropuidgid(pw->pw_name, pw->pw_uid, pw->pw_gid)) return 0;

    if (chdir(pw->pw_dir) == -1) return 0;
    if (!newenv_env("HOME", pw->pw_dir)) return 0;
    if (!newenv_env("USER", pw->pw_name)) return 0;
    if (!newenv_env("LOGNAME", pw->pw_name)) return 0;
    if (!newenv_env("LOGIN", pw->pw_name)) return 0;
    if (!newenv_env("SHELL", pw->pw_shell)) return 0;

    *shell = pw->pw_shell;
#ifdef __ANDROID__
    /*
    At least on some Android devices, pw->pw_shell is hardcoded to "/bin/sh" which does not even exist on the device.
    We do a sanity check here to add a fallback.
    */
    if (access(*shell, R_OK | X_OK) == -1) *shell = "/system/bin/sh";
#endif
    return 1;
}
