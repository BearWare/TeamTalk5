plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "dk.bearware.gui"
    compileSdk = 36

    defaultConfig {
        applicationId = "dk.bearware.gui"
        minSdk = 21
        targetSdk = 36
        versionCode = 145
        versionName = "5.21"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "teamtalk-proguard-rules.txt"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
        isCoreLibraryDesugaringEnabled = true
    }

    splits {
        abi {
            isEnable = true
            reset()
            include("arm64-v8a", "armeabi-v7a", "x86", "x86_64")
            isUniversalApk = true
        }
    }

    buildFeatures {
        buildConfig = true
        viewBinding = true
    }

    androidResources {
        generateLocaleConfig = true
    }

    lint {
        disable += "MissingTranslation"
    }

    buildToolsVersion = "36.1.0"
    ndkVersion = "29.0.14206865"
}

dependencies {
    coreLibraryDesugaring(libs.desugar.jdk)

    implementation(libs.gson)
    implementation(libs.androidx.annotation)
    implementation(libs.androidx.appcompat)
    implementation(libs.androidx.core)
    implementation(libs.androidx.fragment)
    implementation(libs.androidx.media)
    implementation(libs.androidx.preference)
    implementation(libs.androidx.viewpager)
    implementation(libs.material)
    implementation(files("libs/TeamTalk5.jar"))

    testImplementation(libs.junit)
    testImplementation(files("libs/TeamTalk5Test.jar"))

    androidTestImplementation(libs.androidx.test.core)
    androidTestImplementation(libs.androidx.test.runner)
    androidTestImplementation(libs.androidx.test.rules)
    androidTestImplementation(files("libs/TeamTalk5Test.jar"))
}
