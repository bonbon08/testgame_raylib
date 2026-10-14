plugins {
    id("com.android.application")
}

// Google's public test IDs: safe to click, never pay out. Real IDs only come from gradle properties.
val testAdmobAppId = "ca-app-pub-3940256099942544~3347511713"
val testAdmobRewardedId = "ca-app-pub-3940256099942544/5224354917"

fun property(name: String): String? = providers.gradleProperty(name).orNull

// Copies the game's images and sounds into the APK assets (save files stay on the device)
abstract class CopyGameAssets : DefaultTask() {
    @get:InputFiles
    abstract val assetFiles: ConfigurableFileCollection

    @get:OutputDirectory
    abstract val outputDir: DirectoryProperty

    @TaskAction
    fun copy() {
        val dataDir = outputDir.get().asFile.resolve("data")
        dataDir.deleteRecursively()
        dataDir.mkdirs()
        assetFiles.forEach { it.copyTo(dataDir.resolve(it.name), overwrite = true) }
    }
}

val copyGameAssets = tasks.register<CopyGameAssets>("copyGameAssets") {
    assetFiles.from(fileTree(rootProject.file("../data")) { include("*.png", "*.wav") })
}

android {
    namespace = "io.github.bonbon08.jumperchicken"
    compileSdk = 36
    ndkVersion = "29.0.14206865"

    defaultConfig {
        // Google Play Games leaderboard; without these IDs the leaderboard button is hidden
        resValue("string", "play_games_app_id", property("jumperchicken.playGamesAppId") ?: "")
        resValue("string", "leaderboard_id", property("jumperchicken.leaderboardId") ?: "")

        applicationId = "io.github.bonbon08.jumperchicken"
        minSdk = 24
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"

        ndk {
            abiFilters += listOf("arm64-v8a", "armeabi-v7a", "x86_64")
        }

        externalNativeBuild {
            cmake {
                // Use an already downloaded raylib instead of cloning it for every ABI
                property("jumperchicken.raylibDir")?.let { arguments += "-DFETCHCONTENT_SOURCE_DIR_RAYLIB=$it" }
            }
        }
    }

    signingConfigs {
        create("release") {
            property("jumperchicken.keystore")?.let { storeFile = file(it) }
            storePassword = property("jumperchicken.keystorePassword")
            keyAlias = property("jumperchicken.keyAlias")
            keyPassword = property("jumperchicken.keyPassword")
        }
    }

    buildTypes {
        debug {
            // Debug builds always show test ads: clicking your own real ads gets the AdMob account banned
            manifestPlaceholders["admobAppId"] = testAdmobAppId
            resValue("string", "admob_rewarded_id", testAdmobRewardedId)
        }
        release {
            isMinifyEnabled = false
            manifestPlaceholders["admobAppId"] = property("jumperchicken.admobAppId") ?: testAdmobAppId
            resValue("string", "admob_rewarded_id", property("jumperchicken.admobRewardedId") ?: testAdmobRewardedId)
            if (property("jumperchicken.keystore") != null) signingConfig = signingConfigs.getByName("release")
        }
    }

    buildFeatures {
        resValues = true
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "3.31.6"
        }
    }
}

androidComponents {
    onVariants { variant ->
        variant.sources.assets?.addGeneratedSourceDirectory(copyGameAssets, CopyGameAssets::outputDir)
    }
}

dependencies {
    implementation("com.google.android.gms:play-services-ads:25.5.0")
    implementation("com.google.android.ump:user-messaging-platform:4.0.0")
    implementation("com.google.android.gms:play-services-games-v2:22.1.0")
}
