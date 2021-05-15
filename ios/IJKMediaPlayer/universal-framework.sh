# Type a script or drag a script file from your workspace to insert its path.
# Type a script or drag a script file from your workspace to insert its path.
#!/bin/sh

UNIVERSAL_OUTPUTFOLDER=${BUILD_DIR}/${CONFIGURATION}-universal

# make sure the output directory exists
mkdir -p "${UNIVERSAL_OUTPUTFOLDER}"

# Step 1. Build Device and Simulator versions
xcodebuild -target "${TARGET_NAME}" ONLY_ACTIVE_ARCH=NO -configuration ${CONFIGURATION} -sdk iphoneos -arch armv7 -arch armv7s -arch arm64 -project ${PROJECT_DIR}/${PROJECT_NAME}.xcodeproj BUILD_DIR="${BUILD_DIR}" BUILD_ROOT="${BUILD_ROOT}" clean build
xcodebuild -target "${TARGET_NAME}" ONLY_ACTIVE_ARCH=NO -configuration ${CONFIGURATION} -sdk iphonesimulator -project ${PROJECT_DIR}/${PROJECT_NAME}.xcodeproj  BUILD_DIR="${BUILD_DIR}" BUILD_ROOT="${BUILD_ROOT}" clean build


# Step 2. Copy the framework structure (from iphoneos build) to the universal folder
cp -R "${BUILD_DIR}/${CONFIGURATION}-iphoneos/${TARGET_NAME}.framework" "${UNIVERSAL_OUTPUTFOLDER}/"

# Step 3. Copy Swift modules from iphonesimulator build (if it exists) to the copied framework directory
SIMULATOR_SWIFT_MODULES_DIR="${BUILD_DIR}/${CONFIGURATION}-iphonesimulator/${PROJECT_NAME}.framework/Modules/${PROJECT_NAME}.swiftmodule/."
if [ -d "${SIMULATOR_SWIFT_MODULES_DIR}" ]; then
cp -R "${SIMULATOR_SWIFT_MODULES_DIR}" "${UNIVERSAL_OUTPUTFOLDER}/${PROJECT_NAME}.framework/Modules/${PROJECT_NAME}.swiftmodule"
fi

# Step 4. Create universal binary file using lipo and place the combined executable in the copied framework directory
lipo -remove arm64 "${BUILD_DIR}/${CONFIGURATION}-iphonesimulator/${TARGET_NAME}.framework/${TARGET_NAME}" -o "${BUILD_DIR}/${CONFIGURATION}-iphonesimulator/${TARGET_NAME}.framework/${TARGET_NAME}"
lipo -create -output "${UNIVERSAL_OUTPUTFOLDER}/${TARGET_NAME}.framework/${TARGET_NAME}" "${BUILD_DIR}/${CONFIGURATION}-iphonesimulator/${TARGET_NAME}.framework/${TARGET_NAME}" "${BUILD_DIR}/${CONFIGURATION}-iphoneos/${TARGET_NAME}.framework/${TARGET_NAME}"

# Step 5. Convenience step to copy the framework to the project's directory
cp -R "${UNIVERSAL_OUTPUTFOLDER}/${TARGET_NAME}.framework" "${PROJECT_DIR}/../"

rm -v "${PROJECT_DIR}/../CocoaPodsPub/${TARGET_NAME}.tar.xz"
rm -v "${PROJECT_DIR}/../CocoaPodsPub/${TARGET_NAME}.tar.gz"
rm -v -rf "${PROJECT_DIR}/../CocoaPodsPub/${TARGET_NAME}.framework"


cp -v "${PROJECT_DIR}/../../COPYING.LGPLv3" "${PROJECT_DIR}/../CocoaPodsPub/LICENSE"
cd "${PROJECT_DIR}/.."

tar vcfJ "CocoaPodsPub/${TARGET_NAME}.tar.xz" "${TARGET_NAME}.framework"

cd CocoaPodsPub

tree ./
tar -zcvf "${PROJECT_DIR}/../${TARGET_NAME}.tar.gz" ./
mv "${PROJECT_DIR}/../${TARGET_NAME}.tar.gz" "${PROJECT_DIR}/../CocoaPodsPub/${TARGET_NAME}.tar.gz"
mv "${PROJECT_DIR}/../${TARGET_NAME}.framework" "${PROJECT_DIR}/../CocoaPodsPub/${TARGET_NAME}.framework"


# Step 6. Convenience step to open the project's directory in Finder
open "${PROJECT_DIR}/../CocoaPodsPub"
