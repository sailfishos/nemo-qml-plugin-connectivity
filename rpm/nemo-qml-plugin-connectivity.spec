Name:       nemo-qml-plugin-connectivity
Summary:    Connectivity plugin for Sailfish OS
Version:    0.2.11
Release:    1
License:    BSD
URL:        https://github.com/sailfishos/nemo-qml-plugin-connectivity/
Source0:    %{name}-%{version}.tar.bz2
Requires:   connman >= 1.32+git150
Requires:   connman-qt5 >= 1.2.10
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5XmlPatterns)
BuildRequires:  pkgconfig(connman-qt5) >= 1.2.16
BuildRequires:  pkgconfig(qofonoext)
BuildRequires:  pkgconfig(qofono-qt5)

%description
%{summary}.

%package devel
Summary:    Nemo Connectivity development files
Requires:   %{name} = %{version}-%{release}
Requires:   pkgconfig(Qt5DBus)
Requires:   pkgconfig(connman-qt5) >= 1.2.16
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
%qmake5_install

%files
%defattr(-,root,root,-)
%license LICENSE.BSD
%dir %{_libdir}/qt5/qml/Nemo/Connectivity
%{_libdir}/qt5/qml/Nemo/Connectivity/libnemoconnectivity.so
%{_libdir}/qt5/qml/Nemo/Connectivity/plugins.qmltypes
%{_libdir}/qt5/qml/Nemo/Connectivity/qmldir

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
