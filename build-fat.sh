sectors_per_track=32
file_kb=100000
dd if=/dev/zero of=fat.img bs=1024 count=$(expr $file_kb - $file_kb % $sectors_per_track)
mkfs.fat -F 32 fat.img
for f in hdfiles/*; do
    echo copying $f
    mcopy -i fat.img -s $f ::
done