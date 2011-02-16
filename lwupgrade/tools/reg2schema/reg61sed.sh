#!/bin/sh
#
# Copyright (C) Likewise Software. All rights reserved.
#
# Module Name: reg61sed.sh
#
# Abstract:
# Migrate 6.0 registry configuration to 6.X registry
#
# Summary:
# Pstore, DomainTrust, ProviderData, and LinkedCell subkeys are moved
# to conform to the multi-tenancy naming convention.
# Pstore 6.0 entries have been renamed quite a bit in 6.1, so those
# value names are renamed.
#
# Authors: Adam Bernstein (abernstein@likewise.com)
#

TMPSEDSCRIPT="/tmp/upgr61.sed$$"

usage()
{
  echo "ERROR: $1"
  echo "Usage: $0 registry.txt"
  exit 1
}

write_sed_script()
{
  DOMAINNAME=$1
  FQDN=$2
  SHORTDOMAIN=$3

  touch "$TMPSEDSCRIPT"
  if [ $? -ne 0 ]; then
    echo "ERROR: cannot write to $TMPSEDSCRIPT"
    exit 1
  fi

  cat << NNNN > "$TMPSEDSCRIPT"
  #
  # Move/translate pstore default parameters
  #
/\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.Pstore.Default\]/,/^
  s/\\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.Pstore.Default\\]/[HKEY_THIS_MACHINE\\\\Services\\\\lsass\\\\Parameters\\\\Providers\\\\ActiveDirectory\\\\DomainJoin\\\\$DOMAINNAME\\\\Pstore]/
  s/"DomainDnsName"/"DnsDomainName"/
  s/"DomainName"/"NetbiosDomainName"/
  s/"DomainSID"/"DomainSid"/
  s/"HostDnsDomain".*/"Fqdn"="$FQDN"/
  s/"MachineAccount"/"SamAccountName"/
  s/"ClientModifyTimestamp"/"UnixLastChangeTime"/

  #Hack to anchor addition of
  /.*HostName.*/{
    a"AccountFlags"=dword:00000001
    a"KeyVersionNumber"=dword:00000000
    /.*HostName.*/d
  }
  /"CreationTimestamp"/d
  /"SchannelType"/d
  /[^
}

#
# Move Pstore machine password under the subkey of the default joined domain
#
/\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.Pstore.Default.MachinePassword\]/,/^
  /\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.Pstore.Default.MachinePassword\]/a@security = O:SYG:S-1-5-32-544D:(A;;RCSDWDWOKAKRKWKXNRNWNX;;;SY)
  s/\\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.Pstore.Default.MachinePassword]/[HKEY_THIS_MACHINE\\\\Services\\\\lsass\\\\Parameters\\\\Providers\\\\ActiveDirectory\\\\DomainJoin\\\\$DOMAINNAME\\\\Pstore\\\\PasswordInfo]/
  s/"MachinePassword"/"Password"/
  /[^
}

#
# Move DomainTrust load order information under the default joined domain
#
/\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.DomainTrust\]/,/^
  s/\\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.DomainTrust]/[HKEY_THIS_MACHINE\\\\Services\\\\lsass\\\\Parameters\\\\Providers\\\\ActiveDirectory\\\\DomainJoin\\\\$DOMAINNAME\\\\DomainTrust]/
  /[^
}

#
# Move DomainTrust information under the default joined domain
#
/\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.DomainTrust.$SHORTDOMAIN\]/,/^
  s/\\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.DomainTrust.$SHORTDOMAIN]/[HKEY_THIS_MACHINE\\\\Services\\\\lsass\\\\Parameters\\\\Providers\\\\ActiveDirectory\\\\DomainJoin\\\\$DOMAINNAME\\\\DomainTrust\\\\$SHORTDOMAIN]/
  /[^
}

#
# Move ProviderData under the joined DOMAIN_FQDN
#
/\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.ProviderData\]/,/^
  s/\\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.ProviderData]/[HKEY_THIS_MACHINE\\\\Services\\\\lsass\\\\Parameters\\\\Providers\\\\ActiveDirectory\\\\DomainJoin\\\\$DOMAINNAME\\\\ProviderData]/
  /[^
}

#
# Move LinkedCell data under the joined DOMAIN_FQDN
#
/\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.LinkedCell\]/,/^
  s/\\[HKEY_THIS_MACHINE.Services.lsass.Parameters.Providers.ActiveDirectory.LinkedCell]/[HKEY_THIS_MACHINE\\\\Services\\\\lsass\\\\Parameters\\\\Providers\\\\ActiveDirectory\\\\DomainJoin\\\\$DOMAINNAME\\\\LinkedCell]/
  /[^
}
NNNN
}

execute_sed_script()
{
  INFILE=$1
  cat $INFILE | sed -n -f "$TMPSEDSCRIPT"
}


get_host_dns_domain()
{
  infile=$1
  grep '^"HostDnsDomain"=' $infile | \
       sed -e 's|HostDnsDomain.*=||' -e 's|"||g' | \
       tr -d '\r'
}

get_hostname()
{
  infile=$1
  grep '^"HostName"=' $infile | \
        sed -e 's|HostName.*=||' -e 's|"||g' | \
        tr -d '\r'
}


get_short_domain()
{
  infile=$1

  grep '^"ShortDomain"="' $infile | \
  sed -e 's/"ShortDomain"="//' -e 's/"//g' | \
  tr -d '\r'
}


main()
{
  if [ -z "$1" ]; then
    usage "Missing input file name"
  fi
  infile="$1"
  shift

  DOMAIN_NAME_LC=`get_host_dns_domain $infile`
  DOMAIN_NAME=`echo $DOMAIN_NAME_LC | tr 'a-z' 'A-Z'`

  HOSTNAME=`get_hostname $infile`
  HOSTNAME_LC=`echo $HOSTNAME | tr 'A-Z' 'a-z'`

  FQDN="${HOSTNAME_LC}.${DOMAIN_NAME_LC}"
  SHORTDOMAIN=`get_short_domain $infile`

  #
  # Create sed script that modifies the exported 6.0 registry to 6.0+x format
  #
  write_sed_script $DOMAIN_NAME $FQDN $SHORTDOMAIN
  trap "rm -f $TMPSEDSCRIPT; exit 1" 1 2 3 15

  execute_sed_script $infile

  #
  # Cleanup sed script
  #
  rm -f "$TMPSEDSCRIPT"
  exit 0
}

main $@