# MRVL Hal library and our libfm jni
display-hals := libfmhal libfm_jni
include $(call all-named-subdir-makefiles,$(display-hals))
