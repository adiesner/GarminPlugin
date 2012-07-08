Name:           GarminPlugin
Version:        0.3.4
Release:        1
Summary:        Garmin Communicator Plugin port for Linux

License:        GPLv3+

%if 0%{?fedora} > 13
Group:          Other
%endif

%if 0%{?suse_version} >= 1020
Group:          Productivity/Networking/Web/Browsers
%endif


URL:            http://www.andreas-diesner.de/garminplugin/
#Source:         https://github.com/adiesner/GarminPlugin/tarball/V0.3.3
Source:         GarminPlugin-0.3.4.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-build

%if 0%{?suse_version} >= 1020
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  libusb-compat-devel
BuildRequires:  mozilla-xulrunner192-devel
BuildRequires:  tinyxml-devel
BuildRequires:  libgarmintools4-devel
BuildRequires:  zlib-devel
BuildRequires:  pkg-config
%endif

%if 0%{?fedora} > 13
BuildRequires:  gcc
BuildRequires:  gcc-c++
BuildRequires:  tinyxml-devel
BuildRequires:  garmintools-devel
BuildRequires:  libusb-devel
BuildRequires:  zlib-devel
BuildRequires:  xulrunner-devel
# is here because of error "have choice for desktop-notification-daemon needed by libnotify: kdebase-runtime notification-daemon xfce4-notifyd"
BuildRequires:  notification-daemon
%endif


%description
This browser plugin has the same methods and properties as the official 
Garmin Communicator Plugin (http://www8.garmin.com/products/communicator/).
It can be used to transfer GPX files (Geocache Descriptions) to your garmin
device using the official Garmin Javascript API. Its functionality depends on
the device you use. 
- Edge305/Forerunner305: ReadFitnessData, ReadGpsData, No write support
- Edge705/Oregon/Dakota: ReadFitnessData, ReadGpsData, Write Gpx files
- Edge800: ReadFitnessData, Write Gpx/Tcx Files
- Other devices: Executes external command to write Gpx to device


%prep
%setup -q


%build
cd src
%configure --with-ticpp-libdir=%{_libdir} --with-ticpp-incdir=%{_includedir} --with-garmintools-incdir=%{_includedir} --with-garmintools-libdir=%{_libdir}
make


%install
%if 0%{?suse_version} >= 1020
install -D -m 0755 %{_builddir}/%{buildsubdir}/src/npGarminPlugin.so %{buildroot}%{_libdir}/browser-plugins/npGarminPlugin.so
%endif
%if 0%{?fedora} > 13
install -D -m 0755 %{_builddir}/%{buildsubdir}/src/npGarminPlugin.so %{buildroot}%{_libdir}/mozilla/plugins/npGarminPlugin.so
%endif


%clean
rm -rf %{buildroot}


%post -p /sbin/ldconfig
 

%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%doc HISTORY README
%if 0%{?suse_version} >= 1020
%{_libdir}/browser-plugins/*.so
%endif
%if 0%{?fedora} > 13
%{_libdir}/mozilla/plugins/*.so
%endif


%changelog

