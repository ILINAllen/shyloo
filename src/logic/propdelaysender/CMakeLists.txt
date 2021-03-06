INCLUDE_DIRECTORIES(
	"${PROJECT_SOURCE_DIR}/src/public"
	"${PROJECT_SOURCE_DIR}/src/kernel/api"
	"${PROJECT_SOURCE_DIR}/src/logic/interface"
	"${PROJECT_SOURCE_DIR}/src/logic/define"
	"${PROJECT_SOURCE_DIR}/src/sllib/api"
)

SET(PROPDELAYSENDER
	"${PROJECT_SOURCE_DIR}/src/logic/propdelaysender/PropDelaySender.h"
	"${PROJECT_SOURCE_DIR}/src/logic/propdelaysender/PropDelaySender.cpp"
	"${PROJECT_SOURCE_DIR}/src/logic/propdelaysender/main.cpp"
)

SOURCE_GROUP(api FILES ${SL_KERNEL_API})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(interface FILES ${SL_LOGIC_INTERFACE})
SOURCE_GROUP(define FILES ${SL_LOGIC_DEFINE})
SOURCE_GROUP(\\ FILES ${PROPDELAYSENDER})

ADD_LIBRARY(PropDelaySender SHARED
	${PROPDELAYSENDER}
	${SL_KERNEL_API}
	${PUBLIC}
	${SL_LOGIC_INTERFACE}
	${SL_LOGIC_DEFINE}
)

IF(WIN32)
TARGET_LINK_LIBRARIES(PropDelaySender AttrGetter.lib slxml.lib)
ENDIF(WIN32)

IF(UNIX)
TARGET_LINK_LIBRARIES(PropDelaySender libAttrGetter.so)
ENDIF(UNIX)

SET_PROPERTY(TARGET PropDelaySender PROPERTY FOLDER "framework")
