Name:       nemo-qml-plugin-connectivity
Summary:    Connectivity plugin for Nemo Mobile
Version:    0.0.14
Release:    1
Group:      System/Libraries
License:    BSD
URL:        https://git.merproject.org/mer-core/nemo-qml-plugin-connectivity
Source0:    %{name}-%{version}.tar.bz2
Requires:   connman-qt5 >= 1.2.10
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(connman-qt5) >= 1.2.10
BuildRequires:  pkgconfig(qofonoext)
BuildRequires:  pkgconfig(qofono-qt5)

%description
%{summary}.

%package devel
Summary:    Nemo Connectivity development files
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(Qt5DBus)
Requires:   pkgconfig(connman-qt5) >= 1.2.10
Requires:   pkgconfig(qofonoext)
Requires:   pkgconfig(qofono-qt5)

%description devel
%{summary}.

%prep
%setup -q -n %{name}-%{version}

%build
%qmake5 "VERSION=%{version}"
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

# org.nemomobile.connectivity legacy import
mkdir -p %{buildroot}%{_libdir}/qt5/qml/org/nemomobile/connectivity/
ln -sf %{_libdir}/qt5/qml/Nemo/Connectivity/libnemoconnectivity.so %{buildroot}%{_libdir}/qt5/qml/org/nemomobile/connectivity/
sed 's/Nemo.Connectivity/org.nemomobile.connectivity/' < src/plugin/qmldir > %{buildroot}%{_libdir}/qt5/qml/org/nemomobile/connectivity/qmldir

%files
%defattr(-,root,root,-)
%dir %{_libdir}/qt5/qml/Nemo/Connectivity
%{_libdir}/qt5/qml/Nemo/Connectivity/libnemoconnectivity.so
%{_libdir}/qt5/qml/Nemo/Connectivity/plugins.qmltypes
%{_libdir}/qt5/qml/Nemo/Connectivity/qmldir

# org.nemomobile.connectivity legacy import
%dir %{_libdir}/qt5/qml/org/nemomobile/connectivity
%{_libdir}/qt5/qml/org/nemomobile/connectivity/libnemoconnectivity.so
%{_libdir}/qt5/qml/org/nemomobile/connectivity/qmldir

# library
%{_libdir}/libnemoconnectivity.so.*

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/nemo-connectivity
%{_includedir}/nemo-connectivity/*.h
%{_libdir}/libnemoconnectivity.so
%{_libdir}/pkgconfig/nemoconnectivity.pc

%post
/sbin/ldconfig || :

%postun
/sbin/ldconfig || :
