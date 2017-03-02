#
# Makefile for example Arria 10 SoC ARM Cortex A9 Software
# 
# by David M. Koltak  02/18/2016
# 
# ***
# 
# Copyright (c) 2016 David M. Koltak
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of 
# this software and associated documentation files (the "Software"), to deal in the 
# Software without restriction, including without limitation the rights to use, 
# copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in 
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

BOARD ?= soc_a10_devkit_revc

CROSS_COMPILE ?= arm-altera-eabi-
SOCEDS_DEST_ROOT ?= ~/altera/15.1/embedded

SRC     = $(wildcard ./src/common/*.c)
SRC    += $(wildcard ./src/${BOARD}/*.c)

ASM    = $(wildcard ./src/common/*.s)
ASM   += $(wildcard ./src/${BOARD}/*.s)

DFILES = $(sort $(wildcard ./src/${BOARD}/*.rbf)) $(sort $(wildcard ./src/${BOARD}/*.dat))
BLKSZ  = 512

ALTERA_ARCH = soc_a10
HWLIBS_SRC  = $(wildcard ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/src/hwmgr/*.c)
HWLIBS_SRC += $(wildcard ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/src/hwmgr/${ALTERA_ARCH}/*.c)

CFLAGS  = -I ${PWD}/src/include/
CFLAGS += -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include/ 
CFLAGS += -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include/${ALTERA_ARCH}/
CFLAGS += -mcpu=cortex-a9 -D${ALTERA_ARCH} -g -O3 -fno-inline -std=c99

# NOTE: Both IRQ stack allocation/init and vector table configuration are done in startup.c
CFLAGS += -DALT_INT_PROVISION_STACK_SUPPORT=0 -DALT_INT_PROVISION_VECTOR_SUPPORT=0

LFLAGS  = -nostartfiles
LDS     = -T ./src/common/ocram.lds

default:
	@echo ""
	@echo "    USAGE: make {pimage|sdcard|elf|bin|ihex|srec|clean|clean_all}"
	@echo " ------------------------------------------------------------------"
	@echo "             pimage: Image formated for bootrom to load"
	@echo "             sdcard: PImage with concatinated RBFs from BOARD directory"
	@echo "  elf,bin,ihex,srec: Generated with objcopy using specified format"
	@echo "              clean: Remove most of the compiled output files"
	@echo "          clean_all: Also removes all hwlib compiled output files"
	@echo ""
	@echo " NOTE: Set the BOARD env var to one of the ./src/*/ directories"
	@echo "       (default is 'soc_a10_devkit_revb')"
	@echo ""

pimage: ${BOARD}.pimage
        
sdcard: ${BOARD}.sdcard

elf: ${BOARD}.elf

bin: ${BOARD}.bin

ihex: ${BOARD}.ihex
	
srec: ${BOARD}.srec

${BOARD}.pimage: ${BOARD}.bin
	mkpimage -hv 1 -a 256 -o ${BOARD}.pimage ${BOARD}.bin ${BOARD}.bin ${BOARD}.bin ${BOARD}.bin

${BOARD}.sdcard: ${BOARD}.pimage
ifeq ($(strip ${DFILES}),)
	@echo " *** NO Data Files (*.dat|*.rbf) in src/${BOARD}/ directory ***"
	@cp ${BOARD}.pimage ${BOARD}.sdcard
	@echo "> (empty)" | dd bs=${BLKSZ} conv=sync >> ${BOARD}.sdcard
else
	@cd src/${BOARD}/ && stat --printf="> %n [%s]\n" $(notdir ${DFILES}) > dfiles.hdr
	@cp ${BOARD}.pimage ${BOARD}.sdcard
	@echo "dfiles.hdr" && dd if=src/${BOARD}/dfiles.hdr bs=${BLKSZ} conv=sync >> ${BOARD}.sdcard
	@for data_file in ${DFILES} ; do echo $$data_file && dd if=$$data_file bs=${BLKSZ} conv=sync >> ${BOARD}.sdcard ; done
endif
        
${BOARD}.uimage: ${BOARD}.bin
	@echo "TODO: uimage - not sure if we need it since we have pimage, but this is a placeholder for now"

${BOARD}.bin: ${BOARD}.elf
	${CROSS_COMPILE}objcopy -O binary $< ${BOARD}.bin

${BOARD}.ihex: ${BOARD}.elf
	${CROSS_COMPILE}objcopy -O ihex $< ${BOARD}.ihex

${BOARD}.srec: ${BOARD}.elf
	${CROSS_COMPILE}objcopy -O srec $< ${BOARD}.srec

hwlibs.a: ${HWLIBS_SRC:.c=.o}
	${CROSS_COMPILE}ar cr $@ $^

${BOARD}.elf: ${SRC:.c=.o} ${ASM:.s=.o} hwlibs.a
	${CROSS_COMPILE}gcc ${LFLAGS} ${LDS} $^ -o $@
	${CROSS_COMPILE}readelf -h ${BOARD}.elf > ${BOARD}.lst
	${CROSS_COMPILE}readelf -S ${BOARD}.elf >> ${BOARD}.lst
	${CROSS_COMPILE}readelf -s ${BOARD}.elf >> ${BOARD}.lst
	${CROSS_COMPILE}objdump -d ${BOARD}.elf >> ${BOARD}.lst

%.o : %.c
	${CROSS_COMPILE}gcc ${CFLAGS} -c $^ -o $@
        
%.o : %.s
	${CROSS_COMPILE}gcc -c $^ -o $@

clean:
	rm -rf ${BOARD}.pimage ${BOARD}.sdcard src/${BOARD}/dfiles.hdr
	rm -rf ${BOARD}.elf ${BOARD}.bin ${BOARD}.ihex ${BOARD}.srec ${BOARD}.lst
	rm -rf ${SRC:.c=.o} ${ASM:.s=.o}

clean_all: clean
	rm -rf hwlibs.a
	rm -rf ${HWLIBS_SRC:.c=.o}
        
