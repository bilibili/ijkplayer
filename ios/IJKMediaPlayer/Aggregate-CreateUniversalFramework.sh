# Merge Script

# Set bash script to exit immediately if any commands fail.
set -e

# If remnants from a previous build exist, delete them.
if [ -d "${SRCROOT}/build" ]; then
rm -rf "${SRCROOT}/build"
fi


CONFIGURATIONS=("Debug" "Release")

for THE_CONFIGURATION in "${CONFIGURATIONS[@]}"
do :

    # Setup some constants for use later on.
    FRAMEWORK_NAME="IJKMediaFramework"
    OUTPUT_FRAMEWORK_DIR="${PROJECT_DIR}/OUTPUT-FRAMEWORK/${THE_CONFIGURATION}"
    OUTPUT_FRAMEWORK_PATH="${OUTPUT_FRAMEWORK_DIR}/${FRAMEWORK_NAME}.framework"
    mkdir -p "${OUTPUT_FRAMEWORK_DIR}"


    # Build the framework for device and for simulator (using
    # all needed architectures).
    xcodebuild BITCODE_GENERATION_MODE=bitcode OTHER_CFLAGS="-fembed-bitcode" -target "${FRAMEWORK_NAME}" -configuration "${THE_CONFIGURATION}" -arch arm64 only_active_arch=no defines_module=yes -sdk "iphoneos"
    xcodebuild BITCODE_GENERATION_MODE=bitcode OTHER_CFLAGS="-fembed-bitcode" -target "${FRAMEWORK_NAME}" -configuration "${THE_CONFIGURATION}" -arch x86_64 only_active_arch=no defines_module=yes -sdk "iphonesimulator"

    # Remove .framework file if exists from previous run.
    if [ -d "${OUTPUT_FRAMEWORK_PATH}" ]; then
    rm -rf "${OUTPUT_FRAMEWORK_PATH}"
    fi

    # Copy the device version of framework to OutputDir.
    cp -r "${SRCROOT}/build/${THE_CONFIGURATION}-iphoneos/${FRAMEWORK_NAME}.framework" "${OUTPUT_FRAMEWORK_PATH}"

    # Replace the framework executable within the framework with
    # a new version created by merging the device and simulator
    # frameworks' executables with lipo.
    lipo -create -output "${OUTPUT_FRAMEWORK_PATH}/${FRAMEWORK_NAME}" "${SRCROOT}/build/${THE_CONFIGURATION}-iphoneos/${FRAMEWORK_NAME}.framework/${FRAMEWORK_NAME}" "${SRCROOT}/build/${THE_CONFIGURATION}-iphonesimulator/${FRAMEWORK_NAME}.framework/${FRAMEWORK_NAME}"


    rm -rf "{OUTPUT_FRAMEWORK_PATH}/include"
    cp -r "../build/universal/include" "${OUTPUT_FRAMEWORK_PATH}/"

done


# Delete the most recent build.
if [ -d "${SRCROOT}/build" ]; then
rm -rf "${SRCROOT}/build"
fi



open "${PROJECT_DIR}/OUTPUT-FRAMEWORK/"
echo "OUTPUT_FRAMEWORK_DIR: ${PROJECT_DIR}/OUTPUT-FRAMEWORK/"
