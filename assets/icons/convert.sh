#! /bin/bash
APATH="icons_embed.h"
echo "#pragma once" > ${APATH}
echo "namespace assets {" >> ${APATH}

for image in svg/*.svg; do
    OPATH="png/$(basename "${image}" .svg).png"  
    HOPATH="header/$(basename "${image}" .svg)_embed.h"  
    echo ${image}
    if [ ! -f "${OPATH}" ]; then
     inkscape -w 32 -h 32 ${image} -o ${OPATH}
    fi
    
    echo "#include \"${HOPATH}\"" >> ${APATH}
    xxd -i ${OPATH} | sed 's/header//g; s/unsigned/static unsigned/'> ${HOPATH}
done
echo "};" >> ${APATH}
echo "done."
