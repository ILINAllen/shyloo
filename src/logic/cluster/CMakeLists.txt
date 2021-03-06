INCLUDE_DIRECTORIES(
	"${PROJECT_SOURCE_DIR}/src/public"
	"${PROJECT_SOURCE_DIR}/src/kernel/api"
	"${PROJECT_SOURCE_DIR}/src/logic/interface"
	"${PROJECT_SOURCE_DIR}/src/logic/define"
	"${PROJECT_SOURCE_DIR}/src/sllib/api"
)

SET(CLUSTER
	"${PROJECT_SOURCE_DIR}/src/logic/cluster/Cluster.h"
	"${PROJECT_SOURCE_DIR}/src/logic/cluster/Cluster.cpp"
	"${PROJECT_SOURCE_DIR}/src/logic/cluster/main.cpp"
)

SOURCE_GROUP(api FILES ${SL_KERNEL_API})
SOURCE_GROUP(public FILES ${PUBLIC})
SOURCE_GROUP(interface FILES ${SL_LOGIC_INTERFACE})
SOURCE_GROUP(define FILES ${SL_LOGIC_DEFINE})
SOURCE_GROUP(\\ FILES ${CLUSTER})

ADD_LIBRARY(Cluster SHARED
	${CLUSTER}
	${SL_KERNEL_API}
	${PUBLIC}
	${SL_LOGIC_INTERFACE}
	${SL_LOGIC_DEFINE}
)

IF(WIN32)
TARGET_LINK_LIBRARIES(Cluster slxml.lib)
ENDIF(WIN32)

SET_PROPERTY(TARGET Cluster PROPERTY FOLDER "core")