SET( CURL_C
	lib/base64.c
	lib/connect.c
	lib/cookie.c
	lib/dict.c
	lib/easy.c
	lib/escape.c
	lib/file.c
	lib/formdata.c
	lib/ftp.c
	lib/getdate.c
	lib/getenv.c
	lib/getinfo.c
	lib/hash.c
	lib/hostip.c
	lib/http.c
	lib/http_chunks.c
	lib/http_digest.c
	lib/http_ntlm.c
	lib/if2ip.c
	lib/inet_pton.c
	lib/krb4.c
	lib/ldap.c
	lib/llist.c
	lib/md5.c
	lib/memdebug.c
	lib/mprintf.c
	lib/multi.c
	lib/netrc.c
	lib/progress.c
	lib/security.c
	lib/sendf.c
	lib/share.c
	lib/speedcheck.c
	lib/ssluse.c
	lib/strequal.c
	lib/strtok.c
	lib/telnet.c
	lib/timeval.c
	lib/transfer.c
	lib/url.c
	lib/version.c
)

SET( SOURCES
	${CURL_C}
)

INCLUDE_DIRECTORIES(
	include
)

ADD_DEFINITIONS( ${CPP_WARNING_OPTIONS} )
add_library( curl ${SOURCES} )
set_target_properties( curl PROPERTIES PREFIX "")
