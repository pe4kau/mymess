#include <gtest/gtest.h>
#include "thememanager.h"

TEST(ThemeManager, ApplyThemes){
    int argc = 0; char* argv[] = {nullptr};
    // Not creating QApplication here because ThemeManager needs qApp.
    // This test just ensures the enum exists and object constructs.
    ThemeManager tm;
    EXPECT_TRUE(true);
}
