# Typotek specfile
BuildRequires:libqt4-devel freetype2-devel

Name: typotek
Summary: A fonts manager
Version: 0.0svn
Release: 1
License: GPL
Group: Productivity/Fonts
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/build-root-%{name}
Packager: Pierre Marchand
Url: http://www.typotek.net
Provides: typotek
Vendor: œil de pierre
Requires: libqt4 freetype2

%description
A soon powerfull and well designed fonts manager

%prep
%setup -q

%build
qmake -o Makefile typotek.pro
make -j2

%install
# use INSTALL_ROOT instead of DESTDIR
make INSTALL_ROOT=$RPM_BUILD_ROOT install 

%clean
make INSTALL_ROOT=$RPM_BUILD_ROOT uninstall

%files
%defattr(-,root,root,0755)
/usr/bin/typotek

%changelog
#reminder for date : " LC_ALL=C date +"%a %b %d %Y" "
* Wed Oct 24 2007 Pierre Marchand <pierremarc@oep-h.com>
-First package
