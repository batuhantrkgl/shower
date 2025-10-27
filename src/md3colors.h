#ifndef MD3COLORS_H
#define MD3COLORS_H

#include <QColor>
#include <QString>

class MD3Colors
{
public:
    // Material Design 3 Dark Theme Colors
    struct DarkTheme {
        // Primary colors
        static const QColor primary() { return QColor(208, 188, 255); }           // #D0BCFF
        static const QColor onPrimary() { return QColor(56, 30, 114); }          // #381E72
        static const QColor primaryContainer() { return QColor(79, 55, 139); }    // #4F378B
        static const QColor onPrimaryContainer() { return QColor(234, 221, 255); } // #EADDFF
        
        // Secondary colors
        static const QColor secondary() { return QColor(204, 194, 220); }         // #CCC2DC
        static const QColor onSecondary() { return QColor(51, 45, 65); }          // #332D41
        static const QColor secondaryContainer() { return QColor(73, 67, 87); }   // #494357
        static const QColor onSecondaryContainer() { return QColor(232, 222, 248); } // #E8DEF8
        
        // Tertiary colors
        static const QColor tertiary() { return QColor(239, 184, 200); }          // #EFB8C8
        static const QColor onTertiary() { return QColor(73, 37, 50); }           // #492532
        static const QColor tertiaryContainer() { return QColor(99, 59, 72); }    // #633B48
        static const QColor onTertiaryContainer() { return QColor(255, 216, 228); } // #FFD8E4
        
        // Surface colors
        static const QColor surface() { return QColor(16, 16, 20); }              // #101014
        static const QColor onSurface() { return QColor(230, 225, 229); }         // #E6E1E5
        static const QColor surfaceVariant() { return QColor(73, 69, 79); }       // #49454F
        static const QColor onSurfaceVariant() { return QColor(202, 196, 208); }  // #CAC4D0
        
        // Background colors
        static const QColor background() { return QColor(16, 16, 20); }           // #101014
        static const QColor onBackground() { return QColor(230, 225, 229); }      // #E6E1E5
        
        // Surface containers
        static const QColor surfaceContainerLowest() { return QColor(12, 12, 16); } // #0C0C10
        static const QColor surfaceContainerLow() { return QColor(22, 22, 26); }   // #16161A
        static const QColor surfaceContainer() { return QColor(26, 26, 30); }      // #1A1A1E
        static const QColor surfaceContainerHigh() { return QColor(33, 33, 37); }  // #212125
        static const QColor surfaceContainerHighest() { return QColor(43, 43, 48); } // #2B2B30
        
        // Outline colors
        static const QColor outline() { return QColor(147, 143, 153); }           // #938F99
        static const QColor outlineVariant() { return QColor(73, 69, 79); }       // #49454F
        
        // Error colors
        static const QColor error() { return QColor(242, 184, 181); }             // #F2B8B5
        static const QColor onError() { return QColor(96, 20, 16); }              // #601410
        static const QColor errorContainer() { return QColor(140, 29, 24); }      // #8C1D18
        static const QColor onErrorContainer() { return QColor(249, 222, 220); }  // #F9DEDC
    };

    // Elevation and shadow colors
    static const QColor shadow() { return QColor(0, 0, 0); }
    static const QColor scrim() { return QColor(0, 0, 0); }
};

class MD3Typography
{
public:
    // Material Design 3 Typography Scale
    static const QString fontFamily() { return "Roboto"; }
    
    struct DisplayLarge {
        static const int size() { return 57; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 64.0; }
    };
    
    struct DisplayMedium {
        static const int size() { return 45; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 52.0; }
    };
    
    struct DisplaySmall {
        static const int size() { return 36; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 44.0; }
    };
    
    struct HeadlineLarge {
        static const int size() { return 32; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 40.0; }
    };
    
    struct HeadlineMedium {
        static const int size() { return 28; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 36.0; }
    };
    
    struct HeadlineSmall {
        static const int size() { return 24; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 32.0; }
    };
    
    struct TitleLarge {
        static const int size() { return 22; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 28.0; }
    };
    
    struct TitleMedium {
        static const int size() { return 16; }
        static const int weight() { return 500; }
        static const double lineHeight() { return 24.0; }
    };
    
    struct TitleSmall {
        static const int size() { return 14; }
        static const int weight() { return 500; }
        static const double lineHeight() { return 20.0; }
    };
    
    struct BodyLarge {
        static const int size() { return 16; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 24.0; }
    };
    
    struct BodyMedium {
        static const int size() { return 14; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 20.0; }
    };
    
    struct BodySmall {
        static const int size() { return 12; }
        static const int weight() { return 400; }
        static const double lineHeight() { return 16.0; }
    };
    
    struct LabelLarge {
        static const int size() { return 14; }
        static const int weight() { return 500; }
        static const double lineHeight() { return 20.0; }
    };
    
    struct LabelMedium {
        static const int size() { return 12; }
        static const int weight() { return 500; }
        static const double lineHeight() { return 16.0; }
    };
    
    struct LabelSmall {
        static const int size() { return 11; }
        static const int weight() { return 500; }
        static const double lineHeight() { return 16.0; }
    };
};

class MD3Spacing
{
public:
    // Material Design 3 spacing system (8dp base unit)
    static const int spacing0() { return 0; }
    static const int spacing1() { return 4; }     // 0.5 units
    static const int spacing2() { return 8; }     // 1 unit
    static const int spacing3() { return 12; }    // 1.5 units
    static const int spacing4() { return 16; }    // 2 units
    static const int spacing5() { return 20; }    // 2.5 units
    static const int spacing6() { return 24; }    // 3 units
    static const int spacing8() { return 32; }    // 4 units
    static const int spacing10() { return 40; }   // 5 units
    static const int spacing12() { return 48; }   // 6 units
    static const int spacing16() { return 64; }   // 8 units
    static const int spacing20() { return 80; }   // 10 units
    static const int spacing24() { return 96; }   // 12 units
};

class MD3Elevation
{
public:
    // Material Design 3 elevation levels
    static const int level0() { return 0; }
    static const int level1() { return 1; }
    static const int level2() { return 3; }
    static const int level3() { return 6; }
    static const int level4() { return 8; }
    static const int level5() { return 12; }
};

class MD3Shapes
{
public:
    // Material Design 3 shape system
    static const int cornerNone() { return 0; }
    static const int cornerExtraSmall() { return 4; }
    static const int cornerSmall() { return 8; }
    static const int cornerMedium() { return 12; }
    static const int cornerLarge() { return 16; }
    static const int cornerExtraLarge() { return 28; }
    static const int cornerFull() { return 9999; } // Fully rounded
};

#endif // MD3COLORS_H
