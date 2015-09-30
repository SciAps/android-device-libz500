.phony: push format

SHARED_LIB := "../../../../out/target/product/libz500/symbols/system/lib/hw/gps.pcm049.so"

push:
	adb shell stop && \
	sleep 2 && \
	adb push $(SHARED_LIB) /system/lib/hw/ && \
	adb shell start

format:
	find . -type f -name "*.cpp" -exec astyle -A2 -s2 -k1 {} \;
	find . -type f -name "*.h" -exec astyle -A2 -s2 -k1 {} \;

