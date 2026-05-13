// swift-tools-version: 5.9

import PackageDescription

let package = Package(
    name: "TeamTalkKit",
    platforms: [
        .iOS(.v16)
    ],
    products: [
        .library(
            name: "TeamTalkKit",
            targets: ["TeamTalkKit"])
    ],
    targets: [
        .target(
            name: "TeamTalkC",
            publicHeadersPath: "include"),
        .target(
            name: "TeamTalkKit",
            dependencies: ["TeamTalkC"])
    ]
)
