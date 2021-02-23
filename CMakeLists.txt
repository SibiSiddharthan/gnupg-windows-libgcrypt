#[[
   Copyright (c) 2021 Sibi Siddharthan

   Distributed under the MIT license.
   Refer to the LICENSE file at the root directory of the parent project
   for details.
]]

cmake_minimum_required(VERSION 3.12)
project(libgcrypt
		VERSION 1.9.1
		LANGUAGES C ASM)

add_compile_definitions(HAVE_CONFIG_H)

option(DISABLE_ASM "Disable Assembler modules" OFF)

if(CMAKE_C_COMPILER_ID STREQUAL "MSVC" AND NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
	set(DISABLE_ASM ON)
endif()

set(PACKAGE \"${PROJECT_NAME}\")
set(PACKAGE_BUGREPORT \"https://bugs.gnupg.org\")
set(PACKAGE_NAME \"${PROJECT_NAME}\")
set(PACKAGE_STRING \"${PROJECT_NAME}\ ${PROJECT_VERSION}\")
set(PACKAGE_TARNAME \"${PROJECT_NAME}\")
set(PACKAGE_URL \"\")
set(PACKAGE_VERSION \"${PROJECT_VERSION}\")
set(VERSION \"${PROJECT_VERSION}\")
math(EXPR VERSION_HEX "(${PROJECT_VERSION_MAJOR} << 16) + (${PROJECT_VERSION_MINOR} << 8) + ${PROJECT_VERSION_PATCH}" OUTPUT_FORMAT HEXADECIMAL)
set(VERSION_NUMBER ${VERSION_HEX})

#find_package(GPGRT REQUIRED)
include(UtilityFunctions)

include(CheckEndianess)
include(inline)
include(mlock)
include(visibility)

set(BUILD_REVISION \"\")
set(BUILD_TIMESTAMP \"\")
set(GPG_ERR_SOURCE_DEFAULT GPG_ERR_SOURCE_GCRYPT)
set(HAVE_CPU_ARCH_X86 1)
# defaults
set(EGD_SOCKET_NAME \"\")
set(ENABLE_AESNI_SUPPORT 1)
set(ENABLE_AVX2_SUPPORT 1)
set(ENABLE_AVX_SUPPORT 1)
set(ENABLE_DRNG_SUPPORT 1)
set(ENABLE_FORCE_SOFT_HWFEATURES 1)
set(ENABLE_HMAC_BINARY_CHECK 0)
set(ENABLE_JENT_SUPPORT 1)
set(ENABLE_PADLOCK_SUPPORT 1)
set(ENABLE_PCLMUL_SUPPORT 1)
set(ENABLE_SHAEXT_SUPPORT 1)
set(ENABLE_SSE41_SUPPORT 1)

#modules
set(USE_AES 1)
set(USE_ARCFOUR 1)
set(USE_BLAKE2 1)
set(USE_BLOWFISH 1)
set(USE_CAMELLIA 1)
set(USE_CAST5 1)
set(USE_CHACHA20 1)
set(USE_CRC 1)
set(USE_DES 1)
set(USE_DSA 1)
set(USE_ECC 1)
set(USE_ELGAMAL 1)
set(USE_GOST28147 1)
set(USE_GOST_R_3411_12 1)
set(USE_GOST_R_3411_94 1)
set(USE_IDEA 1)
set(USE_MD2 0)
set(USE_MD4 1)
set(USE_MD5 1)
set(USE_RFC2268 1)
set(USE_RMD160 1)
set(USE_RSA 1)
set(USE_SALSA20 1)
set(USE_SCRYPT 1)
set(USE_SEED 1)
set(USE_SERPENT 1)
set(USE_SHA1 1)
set(USE_SHA256 1)
set(USE_SHA3 1)
set(USE_SHA512 1)
set(USE_SM3 1)
set(USE_SM4 1)
set(USE_TIGER 1)
set(USE_TWOFISH 1)
set(USE_WHIRLPOOL 1)

set(LIBGCRYPT_CIPHERS \"arcfour:blowfish:cast5:des:aes:twofish:serpent:rfc2268:seed:camellia:idea:salsa20:gost28147:chacha20\")
set(LIBGCRYPT_DIGESTS \"crc:gostr3411-94::md4:md5:rmd160:sha1:sha256:sha512:sha3:sm3:sm4:tiger:whirlpool:stribog:blake2\")
set(LIBGCRYPT_KDFS \"s2k:pkdf2:scrypt\")
set(LIBGCRYPT_PUBKEY_CIPHERS \"dsa:elgamal:rsa:ecc\")

if(WIN32)
	set(HAVE_VISIBILITY 0)
	set(HAVE_W32_SYSTEM 1)
	set(HAVE_W64_SYSTEM 1)
	set(HAVE_DOSISH_SYSTEM 1)
	set(HAVE_DRIVE_LETTERS 1)
	set(USE_RNDW32 1)
	set(PRINTABLE_OS_NAME \"Windows\")
endif()

if(HAVE_VISIBILITY)
	set(GCRY_USE_VISIBILITY 1)
	add_compile_options(-fvisibility=hidden)
endif()

if(UNIX)
	set(HAVE_DEV_RANDOM 1)
	set(HAVE_PTHREAD 1)
	set(USE_RNDLINUX 1)
	set(NAME_OF_DEV_RANDOM \"/dev/random\")
	set(NAME_OF_DEV_URANDOM \"/dev/urandom\")
	set(PRINTABLE_OS_NAME \"GNU/Linux\")
endif()

check_c_headers(dlfcn.h inttypes.h memory.h stdint.h stdlib.h strings.h string.h
sys/capability.h sys/mman.h sys/msg.h sys/select.h sys/socket.h spawn.h sys/auxv.h
sys/stat.h sys/time.h sys/types.h unistd.h winsock2.h ws2tcpip.h)

if(HAVE_SPAWN_H)
	set(USE_POSIX_SPAWN_FOR_TESTS 1)
endif()

if(HAVE_SYS_CAPABILITY_H)
	set(USE_CAPABILITIES 1)
endif()

check_functions(atexit clock clock_gettime elf_aux_info explicit_bzero explicit_memset fcntl flockfile ftruncate gethrtime getpagesize
getpid getrusage gettimeofday memmove mmap raise rand stpcpy strcasecmp strerror stricmp
strtoul syscall sysconf syslog vprintf wait4 waitpid mlock _doprnt getauxval getentropy)

if(NOT HAVE_VPRINTF AND HAVE__DOPRNT)
	set(HAVE_DOPRNT 1)
endif()

if(WIN32)
	set(CMAKE_EXTRA_INCLUDE_FILES winsock2.h ws2tcpip.h)
else()
	set(CMAKE_EXTRA_INCLUDE_FILES sys/socket.h)
endif()
check_types(pid_t size_t socklen_t uintptr_t uint64_t
"unsigned int" "unsigned long" "unsigned long long" "unsigned short" "void *"
u16 u32 u64 ushort)
unset(CMAKE_EXTRA_INCLUDE_FILES)

if(NOT HAVE_SIZE_T)
	set(size_t "unsigned long long")
endif()

if(NOT HAVE_SOCKLEN_T)
	set(socklen_t int)
endif()

if(NOT HAVE_UINTPTR_T)
	set(uintptr_t "unsigned\ long\ long")
endif()

foreach(f __builtin_bswap32 __builtin_bswap64 __builtin_clz __builtin_clzl __builtin_ctz __builtin_ctzl)
	string(TOUPPER ${f} uf)
	check_compile("Checking for ${f}" "found" "not found" "
		int	main ()
		{
			int x = 0;
			int y = ${f}(x);
			return y;
		}"
		HAVE_${uf})
endforeach()

check_compile("Checking whether GCC assembler supports for CFI directives" "yes" "no" [[
void a()
{
	__asm__(
		".text\n\t"
		"ac_test:\n\t"
		".cfi_startproc\n\t"
		".cfi_remember_state\n\t"
		".cfi_adjust_cfa_offset 8\n\t"
		".cfi_rel_offset 0, 8\n\t"
		".cfi_def_cfa_register 1\n\t"
		".cfi_register 2, 3\n\t"
		".cfi_restore 2\n\t"
		".cfi_escape 0x0f, 0x02, 0x11, 0x00\n\t"
		".cfi_restore_state\n\t"
		".long 0\n\t"
		".cfi_endproc\n\t"
	);
}
int main () {return 0;}]]
HAVE_GCC_ASM_CFI_DIRECTIVES)

check_compile("Checking whether GCC assembler supports for ELF directives" "yes" "no" [[
void a()
{
	__asm__(
		/* Test if ELF directives '.type' and '.size' are supported. */
		".text\n\t"
		"asmfunc:\n\t"
		".size asmfunc,.-asmfunc;\n\t"
		".type asmfunc,STT_FUNC;\n\t"
	);
}
int main () {return 0;}]]
HAVE_GCC_ASM_ELF_DIRECTIVES)

check_compile("Checking whether inline assembly memory barrier is supported" "yes" "no" "
void a(int x)
{
	asm volatile(\"\":::\"memory\");
	asm volatile(\"\":\"+r\"(x)::\"memory\");
}
int main(){return 0;}"
HAVE_GCC_ASM_VOLATILE_MEMORY)

############################
### GCC style attributes ###
############################

check_compile("Checking whether the GCC style aligned attribute is supported" "yes" "no" "
struct { int a; } foo __attribute__ ((aligned (16)));
int main(){return 0;}"
HAVE_GCC_ATTRIBUTE_ALIGNED)

check_compile("Checking whether the GCC style may_alias attribute is supported" "yes" "no" "
typedef struct foo_s { int a; } __attribute__ ((may_alias)) foo_t;
int main(){return 0;}"
HAVE_GCC_ATTRIBUTE_MAY_ALIAS)

check_compile("Checking whether the GCC style packed attribute is supported" "yes" "no" "
struct foolong_s { long b; } __attribute__ ((packed));
struct foo_s { char a; struct foolong_s b; } __attribute__ ((packed));
enum bar {
  FOO = 1 / (sizeof(struct foo_s) == (sizeof(char) + sizeof(long))),
};
int main(){return 0;}"
HAVE_GCC_ATTRIBUTE_PACKED)

############################
########## ABI #############
############################

check_compile("Checking whether compiler supports 'ms_abi' function attribute" "yes" "no" "
int __attribute__ ((ms_abi)) proto(int);
int main(){return 0;}"
HAVE_GCC_ATTRIBUTE_MS_ABI)

check_compile("Checking whether compiler supports 'sysv_abi' function attribute" "yes" "no" "
int __attribute__ ((sysv_abi)) proto(int);
int main(){return 0;}"
HAVE_GCC_ATTRIBUTE_SYSV_ABI)

check_compile("Checking whether default calling convention is 'ms_abi'" "yes" "no" "
void *test(void)
{
	void *(*def_func)(void) = test;
	void *__attribute__((ms_abi))(*msabi_func)(void);
	/* warning on SysV abi targets, passes on Windows based targets */
	msabi_func = def_func;
	return msabi_func;
}
int main(){return 0;}"
HAVE_GCC_DEFAULT_ABI_IS_MS_ABI)

check_compile("Checking whether default calling convention is 'sysv_abi'" "yes" "no" "
void *test(void)
{
	void *(*def_func)(void) = test;
	void *__attribute__((sysv_abi))(*sysvabi_func)(void);
	/* warning on MS ABI targets, passes on SysV ABI targets */
	sysvabi_func = def_func;
	return sysvabi_func;
}
int main(){return 0;}"
HAVE_GCC_DEFAULT_ABI_IS_SYSV_ABI)

############################
##### Inline assembler #####
############################

check_compile("Checking whether your inline assembler supports AVX instructions" "yes" "no" "
void a(void)
{
	__asm__(\"xgetbv; vaesdeclast (%[mem]),%%xmm0,%%xmm7\"::[mem]\"r\"(0):);
}
int main(){return 0;}"
HAVE_GCC_INLINE_ASM_AVX)

check_compile("Checking whether your inline assembler supports AVX2 instructions" "yes" "no" "
void a(void)
{
	__asm__(\"xgetbv; vpbroadcastb %%xmm7,%%ymm1\":::\"cc\");
}
int main(){a();return 0;}"
HAVE_GCC_INLINE_ASM_AVX2)

check_compile("Checking whether your inline assembler supports PCMUL instructions" "yes" "no" "
void a(void) {
	__asm__(\"pclmulqdq $0, %%xmm1, %%xmm3\":::\"cc\");
  }
int main(){a();return 0;}"
HAVE_GCC_INLINE_ASM_PCLMUL)

check_compile("Checking whether your inline assembler supports SHA extension instructions" "yes" "no" [[
void a(void)
{
	__asm__("sha1rnds4 \$0, %%xmm1, %%xmm3\n\t":::"cc");
	__asm__("sha1nexte %%xmm1, %%xmm3\n\t":::"cc");
	__asm__("sha1msg1 %%xmm1, %%xmm3\n\t":::"cc");
	__asm__("sha1msg2 %%xmm1, %%xmm3\n\t":::"cc");
	__asm__("sha256rnds2 %%xmm0, %%xmm1, %%xmm3\n\t":::"cc");
	__asm__("sha256msg1 %%xmm1, %%xmm3\n\t":::"cc");
	__asm__("sha256msg2 %%xmm1, %%xmm3\n\t":::"cc");
}
int main(){a(); return 0;}]]
HAVE_GCC_INLINE_ASM_SHAEXT)

check_compile("Checking whether your inline assembler supports SSE4.1 instructions" "yes" "no" "
void a(void)
{
	int i;
	__asm__(\"pextrd $2, %%xmm0, %[out]\" : [out] \"=m\" (i));
}
int main(){a();return 0;}"
HAVE_GCC_INLINE_ASM_SSE41)

check_compile("Checking whether your inline assembler supports SSE3 instructions" "yes" "no" "
static unsigned char be_mask[16] __attribute__ ((aligned (16))) =
			{ 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };
void a(void)
{
	__asm__(\"pshufb %[mask], %%xmm2\"::[mask]\"m\"(*be_mask):);
}
int main(){a();return 0;}"
HAVE_GCC_INLINE_ASM_SSSE3)

check_compile("Checking whether your inline assembler supports BMI2 instructions" "yes" "no" "
unsigned int a(unsigned int x, unsigned int y)
{
	unsigned int tmp1, tmp2;
	asm (\"rorxl %2, %1, %0\"
		 : \"=r\" (tmp1)
		 : \"rm0\" (x), \"J\" (32 - ((23) & 31)));
	asm (\"andnl %2, %1, %0\"
		 : \"=r\" (tmp2)
		 : \"r0\" (x), \"rm\" (y));
	return tmp1 + tmp2;
}
int main(){a(1,2);return 0;}"
HAVE_GCC_INLINE_ASM_BMI2)

check_compile("Checking whether your assembler is compatible for AMD64 assembly implementations" "yes" "no" [[
int main()
{
	__asm__(
		/* Test if '.type' and '.size' are supported.  */
		/* These work only on ELF targets. */
		"asmfunc:"
		".size asmfunc,.-asmfunc;"
		".type asmfunc,@function;"
		"xorl $(123456789/12345678), %ebp;"
		);
	return 0;
}]]
HAVE_COMPATIBLE_GCC_AMD64_PLATFORM_AS)

check_compile("Checking whether your assembler is compatible for WIN64 assembly implementations" "yes" "no" [[
int main()
{
	__asm__(
              ".globl asmfunc\n\t"
              "asmfunc:"
              "xorq \$(1234), %rbp;"
          );
	return 0;
}]]
HAVE_COMPATIBLE_GCC_WIN64_PLATFORM_AS)

check_compile("Checking for __sync_synchronize" "found" "not found" "
int
main ()
{
	__sync_synchronize();
	return 0;
}"
HAVE_SYNC_SYNCHRONIZE)

if(HAVE_SYS_SELECT_H)
	set(INSERT_SYS_SELECT_H "include <sys/select.h>")
endif()

if(HAVE_SOCKLEN_T)
	set(FALLBACK_SOCKLEN_T "typedef socklen_t gcry_socklen_t;")
endif()

configure_file(libgcrypt-config.h.in config.h)
set(VERSION ${PROJECT_VERSION})
configure_file(src/gcrypt.h.in ${CMAKE_BINARY_DIR}/gcrypt.h @ONLY)
configure_file(src/gcrypt.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/gcrypt.h @ONLY)
if(WIN32)
	if(NOT HAVE_SYS_TIME_H)
		configure_file(sys_time.h.in sys/time.h)
	endif()
endif()

if(NOT DISABLE_ASM)

file(CONFIGURE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mpi/sysdep.h
CONTENT "#define C_SYMBOL_NAME(name) name \n" NEWLINE_STYLE LF)

file(GLOB mpi_asm mpi/amd64/*.S)
foreach(f ${mpi_asm})
# For windows the macro replacement does not preserve tabs for the files defined in func_abi.h
# We manually replace this to fix the issue.
	if(WIN32)
		file(STRINGS ${f} content NEWLINE_CONSUME)
		string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/mpi/amd64/" "" f ${f})
		if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/mpi/${f})
			string(REPLACE "FUNC_ENTRY()"
	"pushq %rsi
	pushq %rdi
	movq %rdx, %rsi
	movq %rcx, %rdi
	movq %r8, %rdx
	movq %r9, %rcx"
		content ${content})
			string(REPLACE "FUNC_EXIT()"
	"popq %rdi
	popq %rsi"
		content ${content})
			file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/mpi/${f} ${content})
		endif()
		list(APPEND CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/mpi/${f}) # Add it here only as the timestamp changes every time when we configure
	else()
		file(COPY ${f} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/mpi)
	endif()
endforeach()

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/mpi/amd64/mpi-asm-defs.h DESTINATION
	${CMAKE_CURRENT_BINARY_DIR}/mpi)

string(APPEND mod_source_info_content "static char mod_source_info[] =\n")
foreach(f ${mpi_asm})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/mpi/" "" nf ${f})
	string(REPLACE "amd64" ":amd64" nf ${nf})
	string(APPEND mod_source_info_content "\"${nf}\"\n")
endforeach()


# Create asm-syntax.h by concatenating i386/syntax.h and amd64/func_abi.h
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/mpi/asm-syntax.h)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/mpi/asm-syntax.h "\n")
	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/mpi/asm-syntax.h "#define ELF_SYNTAX\n")
	file(READ mpi/i386/syntax.h content)
	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/mpi/asm-syntax.h ${content})
	file(READ mpi/amd64/func_abi.h content)
	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/mpi/asm-syntax.h ${content})
endif()
list(APPEND CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/mpi/asm-syntax.h)

else() # ASM disabled

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/mpi/generic/mpi-asm-defs.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/mpi)
file(GLOB mpi_c mpi/generic/*.c)

string(APPEND mod_source_info_content "static char mod_source_info[] =\n")
foreach(f ${mpi_c})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/mpi/" "" nf ${f})
	string(REPLACE "amd64" ":amd64" nf ${nf})
	string(APPEND mod_source_info_content "\"${nf}\"\n")
endforeach()

endif()

# Generate mod-source-info.h
string(APPEND mod_source_info_content ";")
file(CONFIGURE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/mpi/mod-source-info.h
CONTENT "${mod_source_info_content}" NEWLINE_STYLE LF)

set_directory_properties(PROPERTIES ADDITIONAL_CLEAN_FILES "${CLEAN_FILES}")

include_directories(${CMAKE_CURRENT_BINARY_DIR} src ${CMAKE_CURRENT_BINARY_DIR}/mpi mpi)

set(cipher_SOURCES
cipher.c cipher-internal.h
cipher-cbc.c
cipher-cfb.c
cipher-ofb.c
cipher-ctr.c
cipher-aeswrap.c
cipher-ccm.c
cipher-cmac.c
cipher-gcm.c cipher-gcm-intel-pclmul.c
cipher-poly1305.c
cipher-ocb.c
cipher-xts.c
cipher-eax.c
cipher-selftest.c cipher-selftest.h
pubkey.c pubkey-internal.h pubkey-util.c
md.c
mac.c mac-internal.h
mac-hmac.c mac-cmac.c mac-gmac.c mac-poly1305.c
poly1305.c poly1305-internal.h
kdf.c kdf-internal.h
bithelp.h
bufhelp.h
primegen.c
hash-common.c hash-common.h
dsa-common.c rsa-common.c
sha1.h
###
asm-common-aarch64.h
asm-common-amd64.h
asm-common-s390x.h
asm-inline-s390x.h
asm-poly1305-aarch64.h
asm-poly1305-amd64.h
asm-poly1305-s390x.h
arcfour.c arcfour-amd64.S
blowfish.c blowfish-amd64.S blowfish-arm.S
cast5.c cast5-amd64.S cast5-arm.S
chacha20.c chacha20-amd64-ssse3.S chacha20-amd64-avx2.S
chacha20-armv7-neon.S chacha20-aarch64.S
chacha20-ppc.c chacha20-s390x.S
crc.c crc-intel-pclmul.c crc-armv8-ce.c
crc-armv8-aarch64-ce.S
crc-ppc.c
des.c des-amd64.S
dsa.c
elgamal.c
ecc.c ecc-curves.c ecc-misc.c ecc-common.h
ecc-ecdh.c ecc-ecdsa.c ecc-eddsa.c ecc-gost.c ecc-sm2.c
idea.c
gost28147.c gost.h
gostr3411-94.c
md4.c
md5.c
rijndael.c rijndael-internal.h rijndael-tables.h
rijndael-aesni.c rijndael-padlock.c
rijndael-amd64.S rijndael-arm.S
rijndael-ssse3-amd64.c rijndael-ssse3-amd64-asm.S
rijndael-armv8-ce.c rijndael-armv8-aarch32-ce.S
rijndael-armv8-aarch64-ce.S rijndael-aarch64.S
rijndael-ppc.c rijndael-ppc9le.c
rijndael-ppc-common.h rijndael-ppc-functions.h
rijndael-s390x.c
rmd160.c
rsa.c
salsa20.c salsa20-amd64.S salsa20-armv7-neon.S
scrypt.c
seed.c
serpent.c serpent-sse2-amd64.S
sm4.c sm4-aesni-avx-amd64.S sm4-aesni-avx2-amd64.S
serpent-avx2-amd64.S serpent-armv7-neon.S
sha1.c sha1-ssse3-amd64.S sha1-avx-amd64.S sha1-avx-bmi2-amd64.S
sha1-avx2-bmi2-amd64.S sha1-armv7-neon.S sha1-armv8-aarch32-ce.S
sha1-armv8-aarch64-ce.S sha1-intel-shaext.c
sha256.c sha256-ssse3-amd64.S sha256-avx-amd64.S
sha256-avx2-bmi2-amd64.S
sha256-armv8-aarch32-ce.S sha256-armv8-aarch64-ce.S
sha256-intel-shaext.c sha256-ppc.c
sha512.c sha512-ssse3-amd64.S sha512-avx-amd64.S
sha512-avx2-bmi2-amd64.S
sha512-armv7-neon.S sha512-arm.S
sha512-ppc.c sha512-ssse3-i386.c
sm3.c
keccak.c keccak_permute_32.h keccak_permute_64.h keccak-armv7-neon.S
stribog.c
tiger.c
whirlpool.c whirlpool-sse2-amd64.S
twofish.c twofish-amd64.S twofish-arm.S twofish-aarch64.S
twofish-avx2-amd64.S
rfc2268.c
camellia.c camellia.h camellia-glue.c camellia-aesni-avx-amd64.S
camellia-aesni-avx2-amd64.S camellia-arm.S camellia-aarch64.S
blake2.c
blake2b-amd64-avx2.S blake2s-amd64-avx.S)

list(TRANSFORM cipher_SOURCES PREPEND "cipher/")

set(gcrypt_SOURCES
gcrypt-int.h g10lib.h visibility.c visibility.h types.h
gcrypt-testapi.h cipher.h cipher-proto.h
misc.c global.c sexp.c hwfeatures.c hwf-common.h
stdmem.c stdmem.h secmem.c secmem.h
mpi.h missing-string.c fips.c
hmac256.c hmac256.h context.c context.h
ec-context.h
hwf-x86.c)

list(TRANSFORM gcrypt_SOURCES PREPEND "src/")

set(mpi_SOURCES
longlong.h
mpi-add.c
mpi-bit.c
mpi-cmp.c
mpi-div.c
mpi-gcd.c
mpi-internal.h
mpi-inline.h
mpi-inline.c
mpi-inv.c
mpi-mul.c
mpi-mod.c
mpi-pow.c
mpi-mpow.c
mpi-scan.c
mpicoder.c
mpih-div.c
mpih-mul.c
mpih-const-time.c
mpiutil.c
ec.c ec-internal.h ec-ed25519.c)

list(TRANSFORM mpi_SOURCES PREPEND "mpi/")

if(DISABLE_ASM)
	list(APPEND mpi_SOURCES
	mpi/generic/mpih-add1.c
	mpi/generic/mpih-sub1.c
	mpi/generic/mpih-mul1.c
	mpi/generic/mpih-mul2.c
	mpi/generic/mpih-mul3.c
	mpi/generic/mpih-lshift.c
	mpi/generic/mpih-rshift.c)
else()
	list(APPEND mpi_SOURCES
	${CMAKE_CURRENT_BINARY_DIR}/mpi/mpih-add1.S
	${CMAKE_CURRENT_BINARY_DIR}/mpi/mpih-sub1.S
	${CMAKE_CURRENT_BINARY_DIR}/mpi/mpih-mul1.S
	${CMAKE_CURRENT_BINARY_DIR}/mpi/mpih-mul2.S
	${CMAKE_CURRENT_BINARY_DIR}/mpi/mpih-mul3.S
	${CMAKE_CURRENT_BINARY_DIR}/mpi/mpih-lshift.S
	${CMAKE_CURRENT_BINARY_DIR}/mpi/mpih-rshift.S)
endif()

set(random_SOURCES
random.c random.h
rand-internal.h
random-csprng.c
random-drbg.c
random-system.c
#rndjent.c
rndhw.c)

add_library(rndjent_O OBJECT random/rndjent.c)
if(MSVC)
	target_compile_options(rndjent_O PRIVATE /Od)
else()
	target_compile_options(rndjent_O PRIVATE -O0)
endif()
target_link_libraries(rndjent_O GPGRT::GPGRT)

if(WIN32)
	list(APPEND random_SOURCES rndw32.c)
else()
	list(APPEND random_SOURCES rndlinux.c)
endif()

list(TRANSFORM random_SOURCES PREPEND "random/")

if(WIN32)
	list(APPEND gcrypt_SOURCES src/libgcrypt.def)
endif()

add_executable(gost-s-box cipher/gost-s-box.c)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/gost-sb.h
	COMMAND gost-s-box ${CMAKE_CURRENT_BINARY_DIR}/gost-sb.h
	DPENDS gost-s-box WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_custom_target(gen_gost DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/gost-sb.h)

add_library(gcrypt SHARED ${gcrypt_SOURCES} ${cipher_SOURCES} ${random_SOURCES} ${mpi_SOURCES} compat/compat.c)
add_dependencies(gcrypt gen_gost)
target_link_libraries(gcrypt rndjent_O GPGRT::GPGRT)
if(UNIX)
	target_link_libraries(gcrypt pthread)
endif()
set_target_properties(gcrypt PROPERTIES PUBLIC_HEADER ${CMAKE_BINARY_DIR}/gcrypt.h)
set_target_properties(gcrypt PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
set_dll_output_directory(gcrypt ${CMAKE_BINARY_DIR})
add_library(GCRYPT::GCRYPT ALIAS gcrypt)

add_executable(dumpsexp src/dumpsexp.c)
target_link_libraries(dumpsexp gcrypt)

add_executable(mpicalc src/mpicalc.c)
target_link_libraries(mpicalc gcrypt)

add_executable(hmac256 src/hmac256.c)
target_compile_definitions(hmac256 PRIVATE STANDALONE)
target_link_libraries(hmac256 gcrypt)

install(TARGETS gcrypt dumpsexp mpicalc hmac256
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		PUBLIC_HEADER DESTINATION include)

#install(FILES doc/gcrypt.info doc/gcrypt.info-1 doc/gcrypt.info-2 DESTINATION share/info)

include(CTest)
if(BUILD_TESTING)

file(CREATE_LINK ${CMAKE_CURRENT_SOURCE_DIR}/tests/t-ed25519.inp ${CMAKE_CURRENT_BINARY_DIR}/t-ed25519.inp)
set(tests
version t-secmem mpitests t-sexp t-convert
t-mpi-bit t-mpi-point curves t-lock
prime basic keygen pubkey hmac hashtest t-kdf keygrip
fips186-dsa aeswrap pkcs1v2 random dsa-rfc6979
t-ed25519 t-cv25519 t-x448 t-ed448)

foreach(t ${tests})
	add_executable(gcrypt-${t} tests/${t}.c)
	target_link_libraries(gcrypt-${t} gcrypt)
	add_test(NAME gcrypt-${t} COMMAND gcrypt-${t})
	set_target_properties(gcrypt-${t} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests)
	if(WIN32)
		set_tests_properties(gcrypt-${t} PROPERTIES ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}\;${NEW_PATH};GCRYPT_IN_REGRESSION_TEST=1;srcdir=${CMAKE_CURRENT_SOURCE_DIR}/tests")
	else()
		set_tests_properties(gcrypt-${t} PROPERTIES ENVIRONMENT "GCRYPT_IN_REGRESSION_TEST=1;srcdir=${CMAKE_CURRENT_SOURCE_DIR}/tests")
	endif()
endforeach()
add_test(NAME basic-disable-all-hwf COMMAND gcrypt-basic --disable-hwf all)
if(WIN32)
	set_tests_properties(basic-disable-all-hwf PROPERTIES ENVIRONMENT "PATH=${CMAKE_BINARY_DIR}\;${NEW_PATH};GCRYPT_IN_REGRESSION_TEST=1;srcdir=${CMAKE_CURRENT_SOURCE_DIR}/tests")
else()
	set_tests_properties(basic-disable-all-hwf PROPERTIES ENVIRONMENT "GCRYPT_IN_REGRESSION_TEST=1;srcdir=${CMAKE_CURRENT_SOURCE_DIR}/tests")
endif()

# Compile basic.c with full optimization to prevent insufficient registers error
if(MSVC)
	target_compile_options(gcrypt-basic PRIVATE /O2)
else()
	target_compile_options(gcrypt-basic PRIVATE -O3)
endif()

add_executable(benchmark tests/benchmark.c)
target_link_libraries(benchmark gcrypt)
add_executable(bench-slope tests/bench-slope.c)
target_link_libraries(bench-slope gcrypt)

add_custom_target(hashtest256-run COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tests/hashtest --gigs 256 SHA1 SHA256 SHA512 WORKING_DIRECTORY ${CMAKE_BINARY_DIR} USES_TERMINAL)
add_custom_target(benchmark-run COMMAND ${CMAKE_CURRENT_BINARY_DIR}/benchmark WORKING_DIRECTORY ${CMAKE_BINARY_DIR} USES_TERMINAL)
add_custom_target(bench-slope-run COMMAND ${CMAKE_CURRENT_BINARY_DIR}/bench-slope WORKING_DIRECTORY ${CMAKE_BINARY_DIR} USES_TERMINAL)
add_custom_target(big-tests DEPENDS hashtest256-run benchmark-run bench-slope-run)

endif()
