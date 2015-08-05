PRODUCT_PACKAGES := \
    DeskClock \
    Bluetooth \
    Calculator \
    Calendar \
    CertInstaller \
    DrmProvider \
    Email2 \
    Exchange2 \
    FusedLocation \
    Gallery2 \
    InputDevices \
    LatinIME \
    Launcher2 \
    Music \
    MusicFX \
    Provision \
    QuickSearchBox \
    Settings \
    SystemUI \
    CalendarProvider \
    bluetooth-health \
    hostapd \
    wpa_supplicant.conf

PRODUCT_PACKAGES += \
    audio \
    dhcpcd.conf \
    network \
    pand \
    pppd \
    sdptool \
    wpa_supplicant

PRODUCT_PACKAGES += \
    icu.dat

PRODUCT_PACKAGES += \
    librs_jni \
    libvideoeditor_jni \
    libvideoeditor_core \
    libvideoeditor_osal \
    libvideoeditor_videofilters \
    libvideoeditorplayer \

PRODUCT_PACKAGES += \
    audio.primary.default \
    audio_policy.default \
    local_time.default \
    power.default

PRODUCT_PACKAGES += \
    local_time.default

PRODUCT_COPY_FILES := \
        frameworks/av/media/libeffects/data/audio_effects.conf:system/etc/audio_effects.conf

# to flow down to ti-wpan-products.mk
BLUETI_ENHANCEMENT := true

$(call inherit-product-if-exists, frameworks/base/data/fonts/fonts.mk)
$(call inherit-product-if-exists, external/cibu-fonts/fonts.mk)
$(call inherit-product-if-exists, external/lohit-fonts/fonts.mk)
$(call inherit-product-if-exists, external/naver-fonts/fonts.mk)
$(call inherit-product-if-exists, frameworks/base/data/keyboards/keyboards.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/core.mk)

$(call inherit-product, hardware/ti/omap4xxx/omap4.mk)
$(call inherit-product, frameworks/native/build/tablet-dalvik-heap.mk)

$(call inherit-product-if-exists, device/ti/proprietary-open/omap4/ti-omap4-vendor.mk)
#$(call inherit-product, device/ti/proprietary-open/wl12xx/wpan/wl12xx-wpan-fw-products.mk)
$(call inherit-product, vendor/sciaps/wl12xx/wpan/wl12xx-wpan-fw-products.mk)
$(call inherit-product, vendor/sciaps/wl12xx/ti-wl12xx-vendor.mk)
$(call inherit-product-if-exists, hardware/ti/wpan/ti-wpan-products.mk)

DEVICE_PACKAGE_OVERLAYS := device/sciaps/libz500/overlay

TARGET_PROVIDES_INIT_RC := true

# Copy files from our partitions folder to the appropriate outputs
COPY_FILES   := $(foreach I,$(shell find $(LOCAL_PATH)/copyfiles/ -type f | sed 's/.*\/copyfiles\///'), $(LOCAL_PATH)/copyfiles/$(I):$(I))
PRODUCT_COPY_FILES  += $(COPY_FILES)

#Graphics driver
PRODUCT_PACKAGES += \
        pvrsrvkm_sgx540_120.ko

#Wifi Drivers util (only really needed for eng build)
PRODUCT_PACKAGES += \
        calibrator 

#Bluetooth stuff
PRODUCT_PACKAGES += \
	hciconfig \
	hcitool 

#Audio stuff
PRODUCT_PACKAGES += \
	audio.primary.pcm049 \
	tinymix \
	tinyplay \
	tinycap \
	libaudioutils

#USB accessory
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory

# Enable AAC 5.1 decode (decoder)
PRODUCT_PROPERTY_OVERRIDES += \
	media.aac_51_output_enabled=true

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp

#Sensors - Accelerometer
#PRODUCT_PACKAGES += \
#	sensors.pcm049 \
#	sensor.test

# FPGA Version Check
PRODUCT_PACKAGES += \
	fpga.pcm049 \
	board-omap4ksp5012-version.ko

PRODUCT_PACKAGES += \
    lights.pcm049 \
    wilink7.sh \
    busybox \
    playxsvf \
    picprogram \
    com.sciaps.libzhardware \
    LIBZService \
    libserviceJava_jni \
    CMFileManager \
    aLogCat \
    Airdroid \
    Email \
    LIBZAlloyMatch \
    LIBZHome \
    LIBZFactoryMode \
    LIBZUpdater 

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    device/common/gps/gps.conf_US:system/etc/gps.conf


#frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    
PRODUCT_AAPT_CONFIG := normal hdpi xhdpi
PRODUCT_AAPT_PREF_CONFIG := hdpi

PRODUCT_PROPERTY_OVERRIDES += \
    ro.sf.lcd_density=200

PRODUCT_NAME := full_libz500
PRODUCT_DEVICE := libz500
PRODUCT_MODEL := Sciaps LIBZ 500
PRODUCT_MANUFACTURER := Sciaps

PRODUCT_PROPERTY_OVERRIDES += \
        ro.opengles.version=131072 \
        wifi.interface=wlan0

