rm -rf build/zephyr/.config
cp .config build/zephyr/
west build -b qemu_cortex_m3 -t run
