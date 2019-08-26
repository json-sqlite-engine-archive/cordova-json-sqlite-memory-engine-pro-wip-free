# Includes DEPRECATED TARGET CPU PLATFORMS (armeabi mips mips64):
# APP_ABI := all
# with deprecated armeabi platform support now gone:
APP_ABI := armeabi-v7a x86 x86_64 arm64-v8a

# Minimum Android version now 5.0
# Explicit setting ref: https://developer.android.com/ndk/guides/stable_apis
APP_PLATFORM := android-21
