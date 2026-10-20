#include <QLoggingCategory>
#include <cstdlib>

#include <gtest/gtest.h>

// Silences what Qt and its FFmpeg backend print while the tests run (offscreen plugin warnings,
// the FFmpeg dump of the menu music, the probing of hardware video devices). Errors still show.
int main(int argc, char** argv) {
    QLoggingCategory::setFilterRules("*.debug=false\n*.info=false\n*.warning=false");
    // Sends the FFmpeg logs to Qt (category qt.multimedia.ffmpeg) so the rules above filter them
    setenv("QT_FFMPEG_DEBUG", "1", 0);
    setenv("QT_FFMPEG_DECODING_HW_DEVICE_TYPES", "none", 0);
    setenv("QT_FFMPEG_ENCODING_HW_DEVICE_TYPES", "none", 0);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
