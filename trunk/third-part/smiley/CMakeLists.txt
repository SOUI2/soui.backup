include_directories(..)
SET (SMILEY_SRCS
		SSmileyCtrl.cpp
)
ADD_LIBRARY (smiley STATIC ${SMILEY_SRCS})
