Name:           GarminPlugin
Version:        §§§VERSION§§§
Release:        1%{?dist}
Summary:        Garmin Communicator Plugin port for Linux

License:        GPLv2+
Group:          Other
URL:            http://www.andreas-diesner.de/garminplugin/
Source:         http://github.com/adiesner/GarminPlugin/GarminPlugin-§§§VERSION§§§.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildArch:      §§§ARCHITECTURE§§§

BuildRequires:  tinyxml-devel
BuildRequires:  garmintools-devel
BuildRequires:  libusb-devel
BuildRequires:  zlib-devel
BuildRequires:  xulrunner-devel
BuildRequires:  libgcrypt-devel

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
%configure --with-ticpp-libdir=%{_libdir} --with-ticpp-incdir=%{_includedir} --with-garmintools-incdir=%{_includedir} --with-garmintools-libdir=%{_libdir}
make


%install
rm -rf %{buildroot}
mkdir -p %{buildroot}%{_libdir}/mozilla/plugins
cp -p %{_builddir}/%{buildsubdir}/npGarminPlugin.so %{buildroot}%{_libdir}/mozilla/plugins


%clean
rm -rf %{buildroot}


%post
/sbin/ldconfig


%postun -p /sbin/ldconfig


%files
%defattr(-,root,root,-)
%{_libdir}/mozilla/plugins/*.so


%changelog

