# Writed by yijian (eyjian@qq.com, eyjian@gmail.com)

include_directories(.)
include_directories(../include)
include_directories(../include/mooon)
link_directories(../src/net)
link_directories(../src/sys)
link_directories(../src/utils)

# 计算md5工具
add_executable(md5 md5.cpp)
target_link_libraries(md5 libmooon_utils.a)

if (HAVE_LIBSSH2)
	# 远程命令工具
	add_executable(mooon_ssh mooon_ssh.cpp)
	target_link_libraries(mooon_ssh libmooon_net.a libmooon_sys.a libmooon_utils.a libssh2.a libcrypto.a)
	
	# 批量上传工具
	add_executable(mooon_upload mooon_upload.cpp)
	target_link_libraries(mooon_upload libmooon_net.a libmooon_sys.a libmooon_utils.a libssh2.a libcrypto.a)

	# CMAKE_INSTALL_PREFIX
	install(
	        TARGETS mooon_ssh mooon_upload
	        DESTINATION bin
	       )
 endif ()
