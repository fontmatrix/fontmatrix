/*
    SPDX-FileCopyrightText: 2007 Pierre Marchand <pierre@oep-h.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QBitmap>
#include <QCommandLineParser>
#include <QDebug>
#include <QElapsedTimer>
#include <QIcon>
#include <QLocale>
#include <QPainter>
#include <QPixmap>
#include <QSettings>
#include <QSplashScreen>
#include <QStandardPaths>
#include <QStyleHints>
#include <QThread>

#include <KAboutData>
#include <KConfigGroup>
#include <KCrash>
#include <KDBusService>
#include <KIconTheme>
#include <KLocalizedString>
#include <KSharedConfig>
#include <KStyleManager>

#include "fmconfig.h"
#include "fmpaths.h"
#include "mainviewwidget.h"
#include "systray.h"
#include "typotek.h"

#ifdef Q_OS_WIN
// KDECompilerSettings already defines both on the command line
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <cstdio>
#include <windows.h>
#endif

bool __FM_SHOW_FONTLOADED;

namespace
{
/**
 * On Plasma without Plasma's own Qt integration, as in the AppImage, Qt's built-in KDE
 * theme applies the colours of kdeglobals, but KColorSchemeManager, which
 * KIconTheme::initTheme() starts, does not recognise it and puts Breeze or Breeze Dark in
 * their place. A scheme path on the application tells it the colours are set already; it
 * clears the path each time it looks, so the path is set again whenever the system scheme
 * changes, before the manager hears of it.
 */
void keepKdeglobalsColours()
{
    // initTheme() left the icons to a platform theme that does the colours as well
    if (QIcon::themeName() != QLatin1String("KIconEngine"))
        return;
    if (!qEnvironmentVariable("XDG_CURRENT_DESKTOP").split(QLatin1Char(':')).contains(QLatin1String("KDE")))
        return;
    const QString kdeglobals = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QStringLiteral("kdeglobals"));
    if (kdeglobals.isEmpty())
        return;

    const auto markColoursSet = [kdeglobals] {
        qApp->setProperty("KDE_COLOR_SCHEME_PATH", kdeglobals);
    };
    markColoursSet();
    QObject::connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, qApp, markColoursSet);
}
}

/**
 *
 * @param argc
 * @param argv[]
 * @return
 */
int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    // The Windows executable is built for the GUI subsystem, so it has no
    // console of its own and qWarning()/qCritical() are discarded when it is
    // started from cmd.exe. Adopt the parent console when there is one; a
    // launch from Explorer has none and stays windowless.
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE *stream = nullptr;
#ifdef _MSC_VER
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);
#else
        stream = freopen("CONOUT$", "w", stdout);
        stream = freopen("CONOUT$", "w", stderr);
        (void)stream;
#endif
    }
#endif

    // Must be set before QApplication so QSettings picks up the right scope.
    QCoreApplication::setOrganizationName("Fontmatrix");
    QCoreApplication::setOrganizationDomain("io.fontmatrix");
    QCoreApplication::setApplicationName("fontmatrix");

    // On Plasma 6, kded6's kappmenu D-Bus registrar is enabled by default.
    // If no panel widget consumes the exported menu, Qt still hides the local
    // QMenuBar — leaving users with no menu anywhere. Force in-window menus.
    // macOS keeps its system menu via the platform default.
#ifndef Q_OS_MACOS
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar);
#endif

    // Outside Plasma — the AppImage, Windows, another desktop — nothing else themes the
    // application: Breeze icons recoloured to the palette, the colour scheme of the system,
    // and the style the user chose (Fusion where Breeze is not installed, as in the
    // packages Craft makes). Under Plasma's platform theme both calls leave everything to it.
    KIconTheme::initTheme();

    Q_INIT_RESOURCE(application);
    QApplication app(argc, argv);
    KStyleManager::initStyle();
    keepKdeglobalsColours();
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("fontmatrix"), QIcon(QStringLiteral(":/fontmatrix_icon.png"))));

    KLocalizedString::setApplicationDomain("fontmatrix");

    KAboutData aboutData(QStringLiteral("fontmatrix"),
                         QStringLiteral("Fontmatrix"),
                         QStringLiteral("%1.%2.%3").arg(FONTMATRIX_VERSION_MAJOR).arg(FONTMATRIX_VERSION_MINOR).arg(FONTMATRIX_VERSION_PATCH));
    aboutData.setShortDescription(QStringLiteral("Font management application"));
    // Long description preserved from the legacy About dialog (src/messages/about.html).
    aboutData.setOtherText(
        QStringLiteral("<p>Fontmatrix is aimed at adventurous graphic designers and typesetters "
                       "who need to manage hundreds and even thousands of fonts for their work — "
                       "avoiding the need to browse overly long lists in dialogs.</p>"
                       "<p>Basically, Fontmatrix helps you do three things:</p>"
                       "<ul>"
                       "<li>Activating and deactivating your fonts</li>"
                       "<li>Tagging fonts with sets</li>"
                       "<li>Finding suitable fonts by constructing simple or complex queries</li>"
                       "<li>Refining the fonts selection by comparing glyphs in detail</li>"
                       "<li>Generating font \"books\" as PDF.</li>"
                       "</ul>"));
    aboutData.setLicense(KAboutLicense::GPL_V2, KAboutLicense::OrLaterVersions);
    aboutData.setCopyrightStatement(QStringLiteral("© 2007–2026 FontMatrix contributors"));
    aboutData.setHomepage(QStringLiteral("https://github.com/fontmatrix/fontmatrix"));
    aboutData.setBugAddress(QByteArrayLiteral("https://github.com/fontmatrix/fontmatrix/issues"));
    aboutData.setOrganizationDomain(QByteArrayLiteral("io.fontmatrix"));
    // The name of the installed .desktop file; without it KAboutData makes one up from the
    // reversed domain ("fontmatrix.io.fontmatrix") and a Wayland compositor finds no launcher
    // for the window.
    aboutData.setDesktopFileName(QStringLiteral("com.github.fontmatrix.Fontmatrix"));
    // Authors and contributors — preserved from the legacy "The People" tab
    // of the old About dialog (src/messages/about_people.html).
    aboutData.addAuthor(QStringLiteral("Blagovest Petrov"),
                        QStringLiteral("Maintainer since 2026, Qt6 support, Flatpak and improved Windows 11 support, Bulgarian translation"),
                        QStringLiteral("blagovest@petrovs.info"));
    aboutData.addAuthor(QStringLiteral("Pierre Marchand"),
                        QStringLiteral("Initiator of Fontmatrix"),
                        QStringLiteral("pierremarc@oep-h.com"),
                        QStringLiteral("http://oep-h.com"));
    aboutData.addAuthor(QStringLiteral("Mr Docs"),
                        QStringLiteral("Tester, packager and documentation"),
                        QStringLiteral("mrdocs@scribus.info"),
                        QStringLiteral("http://www.scribus.net"));
    aboutData.addAuthor(QStringLiteral("Riku Leino"),
                        QStringLiteral("Systray, minor tweaks, Finnish translation"),
                        QStringLiteral("riku@scribus.info"),
                        QStringLiteral("http://www.tsoots.fi/"));
    aboutData.addAuthor(QStringLiteral("ParagAN"),
                        QStringLiteral("GUI enhancements, Indic fonts"),
                        QStringLiteral("paragn@fedoraproject.org"),
                        QStringLiteral("http://paragn.fedorapeople.org"));
    aboutData.addAuthor(QStringLiteral("Alexandre Prokoudine"),
                        QStringLiteral("Usability, user manual, Russian translation, website"),
                        QStringLiteral("alexandre.prokoudine@gmail.com"),
                        QStringLiteral("http://www.libregraphicsworld.org"));
    aboutData.addAuthor(QStringLiteral("Vladimir Savic"),
                        QStringLiteral("General visual impact, documentation"),
                        QStringLiteral("vladimir.firefly.savic@gmail.com"));
    aboutData.addAuthor(QStringLiteral("Pavel Fric"), QStringLiteral("Czech translation"), QStringLiteral("pavelfric@seznam.cz"));
    aboutData.addCredit(QStringLiteral("Fontmatrix contributors"),
                        QStringLiteral("Original project: https://github.com/fontmatrix/fontmatrix"),
                        QString(),
                        QStringLiteral("https://github.com/fontmatrix/fontmatrix"));
    KAboutData::setApplicationData(aboutData);
    // after KAboutData, which is what the crash handler reports the crash for:
    // DrKonqi where it is installed, a plain backtrace on the console elsewhere
    KCrash::initialize();

    QCommandLineParser parser;
    aboutData.setupCommandLine(&parser);
    // Use parse() instead of process() so early-exit flags are handled below
    // with return 0 rather than ::exit(), which skips Qt thread cleanup and
    // causes "QThreadStorage: entry N destroyed before end of thread" warnings.
    parser.parse(app.arguments());

    if (parser.isSet(QStringLiteral("version"))) {
        fprintf(stdout, "%s %s\n", qPrintable(aboutData.displayName()), qPrintable(aboutData.version()));
        return 0;
    }
    if (parser.isSet(QStringLiteral("help")) || parser.isSet(QStringLiteral("help-all"))) {
        fputs(qPrintable(parser.helpText()), stdout);
        return 0;
    }
    if (parser.isSet(QStringLiteral("author"))) {
        fprintf(stdout, "%s was written by:\n", qPrintable(aboutData.displayName()));
        for (const auto authorsList = aboutData.authors(); const KAboutPerson &person : authorsList) {
            if (!person.emailAddress().isEmpty())
                fprintf(stdout, "    %s <%s>\n", qPrintable(person.name()), qPrintable(person.emailAddress()));
            else
                fprintf(stdout, "    %s\n", qPrintable(person.name()));
        }
        if (!aboutData.bugAddress().isEmpty())
            fprintf(stdout, "Please report bugs to %s.\n", qPrintable(aboutData.bugAddress()));
        return 0;
    }
    // processCommandLine handles any remaining KAboutData-specific flags.
    // It is only reached on a normal GUI launch (no early-exit flags set).
    aboutData.processCommandLine(&parser);

    // One-time QSettings forward-migration from the legacy "Undertype" scope
    // (used by the original Fontmatrix releases) into the current Fontmatrix scope.
    // Runs only when the destination scope is empty. newSettings is a staging
    // area; the KConfig import below pulls everything into the live store.
    {
        QSettings newSettings;
        if (newSettings.allKeys().isEmpty()) {
            QSettings oldSettings(QSettings::defaultFormat(), QSettings::UserScope, QLatin1String("Undertype"), QLatin1String("fontmatrix"));
            const QStringList keys = oldSettings.allKeys();
            for (const QString &key : keys)
                newSettings.setValue(key, oldSettings.value(key));
            if (!keys.isEmpty())
                newSettings.sync();
        }
    }

    // One-time import from QSettings into KConfig on the first launch of a KConfig-enabled build.
    {
        KSharedConfig::Ptr kconf = KSharedConfig::openConfig();
        if (!kconf->group(QStringLiteral("Migration")).hasKey(QStringLiteral("QSettingsImported"))) {
            QSettings qst;
            const QStringList allKeys = qst.allKeys();
            for (const QString &fullKey : allKeys) {
                const int slash = fullKey.indexOf(QLatin1Char('/'));
                const QString group = (slash != -1) ? fullKey.left(slash) : QString{};
                const QString key = (slash != -1) ? fullKey.mid(slash + 1) : fullKey;
                kconf->group(group).writeEntry(key, qst.value(fullKey));
            }
            kconf->group(QStringLiteral("Migration")).writeEntry(QStringLiteral("QSettingsImported"), true);
            kconf->sync();
        }
    }

    // Translation routing is owned by KLocalizedString (KF6 Ki18n). Strings
    // flow through i18n() / i18nc() / i18np() and are looked up in
    // ${KDE_INSTALL_LOCALEDIR}/<lang>/LC_MESSAGES/fontmatrix.mo.
    // setApplicationDomain() is called above next to KAboutData::setApplicationData.

    if (app.arguments().contains("listfonts")) {
        __FM_SHOW_FONTLOADED = true;
    } else {
        __FM_SHOW_FONTLOADED = false;
    }

    // Single-instance guard. KDBusService::Unique aborts a second
    // `fontmatrix` invocation early; the existing process receives an
    // activateRequested signal so it can raise its window. Done before
    // typotek::getInstance() so we don't pay the font-DB init cost twice
    // on a duplicate launch.
    //
    // NoExitOnFailure is required for Windows, where there is no session bus
    // to register with: without it KDBusService calls exit(1) from its
    // constructor and the application terminates before showing a window.
    // It does not weaken the guard on Linux — a duplicate instance exits from
    // a separate branch of KDBusService that activates the running process,
    // and that branch is not governed by this flag.
    //
    // The bus name is the application ID, the only name a Flatpak may own without an
    // --own-name permission. KDBusService builds it from the organization domain and the
    // application name when it is constructed, so both are changed for that moment only:
    // they also name the data directories, and on macOS the settings file.
    const QString organizationDomain = QCoreApplication::organizationDomain();
    const QString applicationName = QCoreApplication::applicationName();
    QCoreApplication::setOrganizationDomain(QStringLiteral("fontmatrix.github.com"));
    QCoreApplication::setApplicationName(QStringLiteral("Fontmatrix"));
    KDBusService dbusService(KDBusService::Unique | KDBusService::NoExitOnFailure);
    QCoreApplication::setOrganizationDomain(organizationDomain);
    QCoreApplication::setApplicationName(applicationName);
    if (!dbusService.isRegistered()) {
        // Expected on Windows; on Linux it means the single-instance guard is
        // inactive for this run.
        qWarning() << "D-Bus service not registered, continuing without the"
                   << "single-instance guard:" << dbusService.errorMessage();
    }

    typotek *mw = typotek::getInstance();

    // Bring the existing window forward when a second invocation is rejected.
    QObject::connect(&dbusService, &KDBusService::activateRequested, mw, [mw](const QStringList & /*args*/, const QString & /*workingDir*/) {
        if (mw->isMinimized())
            mw->setWindowState(mw->windowState() & ~Qt::WindowMinimized);
        mw->show();
        mw->raise();
        mw->activateWindow();
    });

    QSplashScreen theSplash;
    QPixmap theSplashPix(":/fontmatrix_splash.png");
    bool splash = FMConfig::value(QStringLiteral("SplashScreen"), true).toBool();
    if (app.arguments().contains("splash") || splash) {
        QFont spFont;
        spFont.setPointSize(14);
        QPainter p(&theSplashPix);
        p.setFont(spFont);
        p.setPen(Qt::white);
        QString vString(QString("%1.%2.%3").arg(FONTMATRIX_VERSION_MAJOR).arg(FONTMATRIX_VERSION_MINOR).arg(FONTMATRIX_VERSION_PATCH));
        p.drawText(theSplashPix.width() / 4, theSplashPix.height() / 3, vString);
        p.end();

        spFont.setPointSize(9);
        theSplash.setPixmap(theSplashPix);
        theSplash.setFont(spFont);
        QObject::connect(mw, &typotek::relayStartingStepOut, &theSplash, &QSplashScreen::showMessage, Qt::DirectConnection);
    }

    QElapsedTimer splashTimer;
    if (splash) {
        theSplash.show();
        splashTimer.start();
    }

    mw->initMatrix();

    if ((typotek::getInstance()->getSystray()) && (typotek::getInstance()->getSystray()->isVisible())
        && (FMConfig::value(QStringLiteral("Systray/CloseToTray"), true).toBool())) {
        if (!FMConfig::value(QStringLiteral("Systray/StartToTray"), false).toBool())
            mw->show();
        else
            mw->hide();
    } else
        mw->show();

    LazyInit lazyInit;
    lazyInit.start(QThread::LowestPriority);

    mw->postInit();

    if (splash) {
        const int minSplashMs = 1500;
        qint64 remaining = minSplashMs - splashTimer.elapsed();
        if (remaining > 0)
            QThread::msleep(static_cast<unsigned long>(remaining));
        theSplash.finish(mw);
    }

    return app.exec();
}
