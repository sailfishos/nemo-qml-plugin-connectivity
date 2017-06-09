Name:       nemo-qml-plugin-connectivity
Summary:    Connectivity plugin for Nemo Mobile
Version:    0.0.7
Release:    1
Group:      System/Libraries
License:    BSD
URL:        https://git.merproject.org/mer-core/nemo-qml-plugin-connectivity
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(connman-qt5)

%description
%{summary}.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

# org.nemomobile.connectivity legacy import
mkdir -p %{buildroot}%{_libdir}/qt5/qml/org/nemomobile/connectivity/
ln -sf %{_libdir}/qt5/qml/Nemo/Connectivity/libnemoconnectivity.so %{buildroot}%{_libdir}/qt5/qml/org/nemomobile/connectivity/
sed 's/Nemo.Connectivity/org.nemomobile.connectivity/' < src/qmldir > %{buildroot}%{_libdir}/qt5/qml/org/nemomobile/connectivity/qmldir

%files
%defattr(-,root,root,-)
%dir %{_libdir}/qt5/qml/Nemo/Connectivity
%{_libdir}/qt5/qml/Nemo/Connectivity/libnemoconnectivity.so
%{_libdir}/qt5/qml/Nemo/Connectivity/qmldir

# org.nemomobile.connectivity legacy import
%dir %{_libdir}/qt5/qml/org/nemomobile/connectivity
%{_libdir}/qt5/qml/org/nemomobile/connectivity/libnemoconnectivity.so
%{_libdir}/qt5/qml/org/nemomobile/connectivity/qmldir
