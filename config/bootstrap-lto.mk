# This option enables LTO for stage2 and stage3.  It requires lto to
# be enabled for stage1 with --enable-stage1-languages.

STAGE2_CFLAGS += -flto=jobserver -fuse-linker-plugin -frandom-seed=1
STAGE3_CFLAGS += -flto=jobserver -fuse-linker-plugin -frandom-seed=1
