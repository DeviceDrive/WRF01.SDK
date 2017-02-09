# WRF.SDK

This library is packaged both as an generic C / C++ library and as an Arduino library.
To use the generic library, use the files located in WRF.SDK
File headers and code is documented.

To use the Arduino library, make sure to run the "CreateLibrary.bat" script, and move the
library folder to your local Arduino libraries folder on your computer. On Windows, this folder is
located under "My Documents"\Arduino\libraries by default. 

If you are using Unix / Mac OSX you can use the "CreateLibrary.sh".
The file is written in a windows environment, so if it failes, check for <CR> <LF> charaters in the file. Unix systems only use <LF>


The SDK.NRF folder contains an example on how to use the generic library on a Nordic Semiconductor micro controller.
Be advised that you need to setup your environment beforehand and edit the sdk_config.h in the NRF setup to suit your needs.