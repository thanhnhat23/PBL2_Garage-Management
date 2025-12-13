#ifndef STYLEHELPER_H
#define STYLEHELPER_H

#include <QString>

class StyleHelper {
public:
    static QString getButtonStyle() {
     return "QPushButton {"
         "   background-color: #2563eb;"
         "   color: #f8fafc;"
         "   font-size: 12px;"
         "   font-weight: 600;"
         "   border: 1px solid #1d4ed8;"
         "   border-radius: 8px;"
         "   padding: 8px;"
         "   min-height: 25px;"
         "}"
         "QPushButton:hover {"
         "   background-color: #1d4ed8;"
         "}"
         "QPushButton:pressed {"
         "   background-color: #1e40af;"
         "}";
    }
    
    static QString getSecondaryButtonStyle() {
     return "QPushButton {"
         "   background-color: #1f2937;"
         "   color: #e5e7eb;"
         "   font-size: 12px;"
         "   border: 1px solid #334155;"
         "   border-radius: 8px;"
         "   padding: 8px;"
         "   min-height: 25px;"
         "}"
         "QPushButton:hover {"
         "   background-color: #111827;"
         "}"
         "QPushButton:pressed {"
         "   background-color: #0f172a;"
         "}";
    }
    
    static QString getDangerButtonStyle() {
     return "QPushButton {"
         "   background-color: rgba(220, 38, 38, 0.15);"
         "   color: #ef4444;"
         "   font-size: 12px;"
         "   font-weight: 600;"
         "   border: 1px solid #7f1d1d;"
         "   border-radius: 8px;"
         "   padding: 8px;"
         "   min-height: 25px;"
         "}"
         "QPushButton:hover {"
         "   background-color: rgba(220, 38, 38, 0.25);"
         "}"
         "QPushButton:pressed {"
         "   background-color: rgba(220, 38, 38, 0.35);"
         "}";
    }
    
    static QString getSuccessButtonStyle() {
     return "QPushButton {"
         "   background-color: rgba(34, 197, 94, 0.15);"
         "   color: #22c55e;"
         "   font-size: 12px;"
         "   font-weight: 600;"
         "   border: 1px solid #15803d;"
         "   border-radius: 8px;"
         "   padding: 8px;"
         "   min-height: 25px;"
         "}"
         "QPushButton:hover {"
         "   background-color: rgba(34, 197, 94, 0.25);"
         "}"
         "QPushButton:pressed {"
         "   background-color: rgba(34, 197, 94, 0.35);"
         "}";
    }
    
    static QString getPrimaryButtonStyle() {
     return "QPushButton {"
         "   background-color: rgba(59, 130, 246, 0.15);"
         "   color: #3b82f6;"
         "   font-size: 12px;"
         "   font-weight: 600;"
         "   border: 1px solid #1e40af;"
         "   border-radius: 8px;"
         "   padding: 8px;"
         "   min-height: 25px;"
         "}"
         "QPushButton:hover {"
         "   background-color: rgba(59, 130, 246, 0.25);"
         "}"
         "QPushButton:pressed {"
         "   background-color: rgba(59, 130, 246, 0.35);"
         "}";
    }
    
    static QString getTableStyle() {
     return "QTableWidget {"
         "   border: 1px solid #1f2937;"
         "   border-radius: 10px;"
         "   background-color: #0b1220;"
         "   gridline-color: #1f2937;"
         "   selection-background-color: #1d4ed8;"
         "   selection-color: #e5e7eb;"
         "   color: #e5e7eb;"
         "   font-size: 13px;"
         "}"
         "QTableWidget::item {"
         "   padding: 8px;"
         "   background: transparent;"
         "}"
        "QTableWidget::item:selected:active {"
        "   background: #2563eb;"
        "   color: #f8fafc;"
        "}"
        "QTableWidget::item:selected:!active {"
        "   background: #1f2937;"
        "   color: #e5e7eb;"
        "}"
         "QTableWidget::item:alternate {"
         "   background: #0f172a;"
         "}"
         "QHeaderView::section {"
         "   background-color: #111827;"
         "   color: #e5e7eb;"
         "   font-weight: 700;"
         "   border: 1px solid #1f2937;"
         "   padding: 10px;"
         "}"
         "QTableCornerButton::section {"
         "   background: #111827;"
         "   border: 1px solid #1f2937;"
         "}";
    }
    
    static QString getTabStyle() {
     return "QTabWidget::pane {"
         "   border: 1px solid #1f2937;"
         "   border-radius: 10px;"
         "   background: #0b1220;"
         "   top: -1px;"
         "}"
         "QTabBar::tab {"
         "   background: #0f172a;"
         "   color: #9ca3af;"
         "   padding: 12px 20px;"
         "   margin-right: 4px;"
         "   border-top-left-radius: 10px;"
         "   border-top-right-radius: 10px;"
         "   font-weight: 700;"
         "}"
         "QTabBar::tab:selected {"
         "   background: #111827;"
         "   color: #e5e7eb;"
         "   border: 1px solid #1f2937;"
         "   border-bottom: 2px solid #2563eb;"
         "}"
         "QTabBar::tab:hover {"
         "   background: #111827;"
         "}";
    }

    // Input fields styling for search bars and text fields
    static QString getInputStyle() {
     return "QLineEdit {"
         "   background: #0b1220;"
         "   color: #e5e7eb;"
         "   border: 1px solid #1f2937;"
         "   border-radius: 8px;"
         "   padding: 8px 12px;"
         "   font-size: 14px;"
         "}"
         "QLineEdit:focus {"
         "   border: 1px solid #2563eb;"
         "   box-shadow: 0 0 0 1px #2563eb;"
         "}";
    }
};

#endif
