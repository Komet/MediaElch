#
# spec file for package mediaelch
#

Name:           MediaElch
Version:        2.8.6
Release:        1%{?dist}
License:        LGPL-2.1+
Summary:        A Media Manager for Kodi
URL:            https://github.com/Komet/MediaElch
Group:          Productivity/Multimedia/Other
Requires:       libmediainfo0 ffmpeg
Source0:        %{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

BuildRequires:  gcc7
BuildRequires:  gcc7-c++
BuildRequires:  libqt5-qtbase-devel
BuildRequires:  libQt5Multimedia-devel
BuildRequires:  libqt5-qttools-devel
BuildRequires:  libmediainfo-devel
BuildRequires:  libpulse-devel

%description
MediaElch is a MediaManager for Kodi. Information about Movies, TV Shows, Concerts and
Music are stored as nfo files. Fanarts are downloaded automatically from fanart.tv.
Using the nfo generator, MediaElch can be used with other MediaCenters as well.

%prep
%setup -n MediaElch
export CC=/usr/bin/gcc-7
export CXX=/usr/bin/g++-7
gcc --version
g++ --version
qmake-qt5 --version
qmake-qt5 CONFIG+=release

%build
make %{?jobs:-j%jobs}

%install
export INSTALL_ROOT="%{buildroot}"
make install

%files
%defattr(-,root,root,-)
%doc README.md
%license COPYING
%{_bindir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/pixmaps/%{name}.png

%changelog
