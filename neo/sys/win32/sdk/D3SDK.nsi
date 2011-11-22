SetCompressor lzma

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "Doom 3 SDK"
!define PRODUCT_VERSION "1.3.1"
!define PRODUCT_PUBLISHER "id Software"
!define PRODUCT_WEB_SITE "http://www.iddevnet.com"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"

; MUI Pages
!insertmacro MUI_PAGE_WELCOME
!define MUI_LICENSEPAGE_RADIOBUTTONS
!insertmacro MUI_PAGE_LICENSE "Doom3_SDK\EULA.Development Kit.rtf"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "D3_${PRODUCT_VERSION}_SDK.exe"
InstallDir "C:\Doom3_SDK\"
ShowInstDetails show

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /R "Doom3_SDK\*.*"
SectionEnd

Section -Post
SectionEnd

