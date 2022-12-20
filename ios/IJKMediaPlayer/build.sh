rm -rf build
xcrun xcodebuild -scheme IJKMediaFrameworkWithSSL -target IJKMediaFrameworkWithSSL -configuration Debug -sdk iphonesimulator -destination "platform=iOS Simulator,name=Any iOS Simulator Device" build -derivedDataPath build && \
xcrun xcodebuild -scheme IJKMediaFrameworkWithSSL -target IJKMediaFrameworkWithSSL -configuration Release -sdk iphoneos build -derivedDataPath build && \
xcrun xcodebuild -create-xcframework -framework build/Build/Products/Debug-iphonesimulator/IJKMediaFrameworkWithSSL.framework -framework build/Build/Products/Release-iphoneos/IJKMediaFrameworkWithSSL.framework -output build/Build/Products/IJKMediaFrameworkWithSSL.xcframework
