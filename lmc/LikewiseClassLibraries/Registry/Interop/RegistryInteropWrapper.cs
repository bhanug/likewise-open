/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

using System;
using System.Collections.Generic;
using System.Text;
using Likewise.LMC.Utilities;
using System.Runtime.InteropServices;

using System.Security.Principal;
using System.Security.Permissions;
using Microsoft.Win32;

namespace Likewise.LMC.Registry
{
    public class RegistryInteropWrapper
    {
        public enum LogonType : int
        {
            //'This logon type is intended for batch servers, where processes may be executing on behalf of a user without
            //'their direct intervention. This type is also for higher performance servers that process many plaintext
            //'authentication attempts at a time, such as mail or Web servers.
            //'The LogonUser function does not cache credentials for this logon type.
            LOGON32_LOGON_BATCH = 4,

            //'This logon type is intended for users who will be interactively using the computer, such as a user being logged on
            //'by a terminal server, remote shell, or similar process.
            //'This logon type has the additional expense of caching logon information for disconnected operations;
            //'therefore, it is inappropriate for some client/server applications,
            //'such as a mail server.
            LOGON32_LOGON_INTERACTIVE = 2,

            //'This logon type is intended for high performance servers to authenticate plaintext passwords.
            //'The LogonUser function does not cache credentials for this logon type.
            LOGON32_LOGON_NETWORK = 3,

            //'This logon type preserves the name and password in the authentication package, which allows the server to make
            //'connections to other network servers while impersonating the client. A server can accept plaintext credentials
            //'from a client, call LogonUser, verify that the user can access the system across the network, and still
            //'communicate with other servers.
            //'NOTE: Windows NT:  This value is not supported.
            LOGON32_LOGON_NETWORK_CLEARTEXT = 8,

            //'This logon type allows the caller to clone its current token and specify new credentials for outbound connections.
            //'The new logon session has the same local identifier but uses different credentials for other network connections.
            //'NOTE: This logon type is supported only by the LOGON32_PROVIDER_WINNT50 logon provider.
            //'NOTE: Windows NT:  This value is not supported.
            LOGON32_LOGON_NEW_CREDENTIALS = 9,

            //'Indicates a service-type logon. The account provided must have the service privilege enabled.
            LOGON32_LOGON_SERVICE = 5,

            //'This logon type is for GINA DLLs that log on users who will be interactively using the computer.
            //'This logon type can generate a unique audit record that shows when the workstation was unlocked.
            LOGON32_LOGON_UNLOCK = 7
        }

        public enum LogonProvider : int
        {
            /// <summary>
            /// Use the standard logon provider for the system.
            /// The default security provider is negotiate, unless you pass NULL for the domain name and the user name
            /// is not in UPN format. In this case, the default provider is NTLM.
            /// NOTE: Windows 2000/NT:   The default security provider is NTLM.
            /// </summary>
            LOGON32_PROVIDER_DEFAULT = 0,
            LOGON32_PROVIDER_WINNT50,
            LOGON32_PROVIDER_WINNT40
        }

        public static uint MAX_REG_KEYNAME_SIZE = ((102 * 1024) * 1024);

        public static int MAX_KEY_LENGTH = 255;
        public static int MAX_VALUE_LENGTH = 1024;

        //Registry datatypes
        public static long REG_DWORD = 4;
        public static long REG_DWORD_BIG_ENDIAN = 5;

        private static string sHostName = string.Empty;

        #region ADVAPI32 Wrapper implementation

        public static IntPtr OpenHandle()
        {
            IntPtr pHandle;

            Logger.Log("RegistryInterop.LwRegOpenServer(out pHandle) is getting called", Logger.RegistryViewerLoglevel);

            int dwError = RegistryInterop.LwRegOpenServer(out pHandle);

            Logger.Log(string.Format("RegistryInterop.LwRegOpenServer(out pHandle = {0}) is called ", (pHandle == IntPtr.Zero) ? "zero" : pHandle.ToInt32().ToString()), Logger.RegistryViewerLoglevel);

            if (dwError != 0)
            {
                pHandle = IntPtr.Zero;
                throw new Exception(String.Format(
                "Error: OpenFileHandle [Code:{0}]", dwError));
            }
            return pHandle;
        }


        public static int RegServerOpen(out IntPtr hRegConnection)
        {
            IntPtr handle_t = IntPtr.Zero;

            int iResult = RegistryInterop.LwRegOpenServer(out handle_t);
            if (iResult != 0)
            {
                hRegConnection = IntPtr.Zero;
                Logger.Log("RegistryInteropWrapper.RegOpenServer ret = {0}" + iResult.ToString(), Logger.RegistryViewerLoglevel);
                return iResult;
            }

            Logger.Log("RegistryInteropWrapper.RegOpenServer is successfull handle = {0}" + handle_t.ToInt32(), Logger.RegistryViewerLoglevel);

            hRegConnection = handle_t;

            return iResult;
        }


        public static int RegCloseServer(out IntPtr hRegConnection)
        {
            hRegConnection = IntPtr.Zero;

            int iResult = RegistryInterop.LwRegCloseServer(hRegConnection);
            if (iResult != 0)
            {
                Logger.Log(string.Format("RegistryInteropWrapper.RegCloseServer(hRegConnection={0}) :ret = {1}", hRegConnection.ToInt32().ToString(), iResult.ToString()), Logger.RegistryViewerLoglevel);
                return iResult;
            }

            Logger.Log("RegistryInteropWrapper.RegCloseServer is successfull handle = {0}" + hRegConnection.ToInt32().ToString(), Logger.RegistryViewerLoglevel);

            return iResult;
        }

        public static int ApiRegEnumRootKeys(IntPtr hRegConnection, out string[] sRootKeyEnum, out int pRootKeyEnumCount)
        {
            int iResult;
            pRootKeyEnumCount = 0;

            IntPtr pRootKeyEnum = IntPtr.Zero;

            Logger.Log("RegistryInteropWrapper.ApiRegEnumRootKeysW(hRegConnection = " + hRegConnection.ToString(), Logger.RegistryViewerLoglevel);

            iResult = RegistryInterop.LwRegEnumRootKeysW(hRegConnection, out pRootKeyEnum, out pRootKeyEnumCount);

            if (iResult != 0)
            {
                Logger.Log("RegistryInteropWrapper.RegEnumRootKeysW( ret = " + iResult.ToString(), Logger.RegistryViewerLoglevel);
                sRootKeyEnum = null;
                return iResult;
            }

            Logger.Log(string.Format("RegistryInteropWrapper.RegEnumRootKeysW is successfull with the count pRootKeyEnumCount = " + pRootKeyEnumCount), Logger.RegistryViewerLoglevel);

            IntPtr[] pNewRootKeyEnum = new IntPtr[pRootKeyEnumCount];
            Marshal.Copy(pRootKeyEnum, pNewRootKeyEnum, 0, pRootKeyEnumCount);
            sRootKeyEnum = new string[pRootKeyEnumCount];

            for (int idx = 0; idx < pRootKeyEnumCount; idx++)
                sRootKeyEnum[idx] = Marshal.PtrToStringUni(pNewRootKeyEnum[idx]);

            return iResult;
        }

        public static int ApiRegOpenKeyExW(IntPtr hRegConnection, IntPtr hKey, string sKeyname, out IntPtr phkResult)
        {
            int iResult = 0;
			IntPtr phResult;

            try
            {
                Logger.Log(string.Format("RegistryInteropWrapper.RegOpenKeyExW(hRegConnection={0}, hKey={1} sSubkey={2}),  is called ",
                          hRegConnection.ToInt32().ToString(), hKey.ToInt32().ToString(), sKeyname, Logger.RegistryViewerLoglevel));

                iResult = RegistryInterop.LwRegOpenKeyExW(hRegConnection,
                                                hKey,
                                                sKeyname,
                                                0,
                                                RegistryApi.RegSAM.AllAccess,
                                                out phResult);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegOpenKeyExW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
					 phkResult = IntPtr.Zero;
                }

				phkResult = phResult;

                Logger.Log(string.Format("RegistryInterop.LwRegOpenKeyExW() is returns iResult={0}, \n" +
                                "out phkResult={1}", iResult.ToString(), phkResult.ToInt32().ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegOpenKeyExW", ex);
				phkResult = IntPtr.Zero;
            }

            return iResult;
        }

        public static int ApiRegEnumKeyEx(
                                IntPtr handle_t,
                                IntPtr pParentKey,
                                uint dwSubKeyCount,
                               // ulong dwMaxSubKeyLen,
                                out List<RegistryEnumKeyInfo> enumKeys)
        {
            uint idx = 0;
            int iResult = 0;
            enumKeys = new List<RegistryEnumKeyInfo>();

            try
            {
                while (idx < dwSubKeyCount)
                {
                    uint pcClass = (uint)MAX_KEY_LENGTH;
                    string sValue = string.Empty;
                    RegistryApi.FILETIME lastWriteTime = null;
                    RegistryEnumKeyInfo KeyInfo = new RegistryEnumKeyInfo();
                    StringBuilder pNameBuf = new StringBuilder(MAX_KEY_LENGTH);
                    StringBuilder pClassBuf = new StringBuilder(MAX_KEY_LENGTH);
                    uint pcName = (uint)MAX_KEY_LENGTH;

                    KeyInfo.initializeToNull();

                    Logger.Log(string.Format("RegistryInteropWrapper.RegEnumKeyExW(handle_t={0}, pParentKey={1}",
                        handle_t.ToInt32().ToString(), pParentKey.ToInt32().ToString()), Logger.RegistryViewerLoglevel);

                    Logger.Log(string.Format("RegistryInteropWrapper.RegEnumKeyExW: keyname.Length={0}",
                        MAX_KEY_LENGTH.ToString(), Logger.RegistryViewerLoglevel));

                    iResult = RegistryInterop.LwRegEnumKeyExW(
                                            handle_t,
                                            pParentKey,
                                            idx,
                                            pNameBuf,
                                            ref pcName,
                                            IntPtr.Zero,
                                            pClassBuf,
                                            ref pcClass,
                                            null);
                    if (iResult != 0)
                    {
                        Logger.Log(string.Format("RegistryInteropWrapper.RegEnumKeyExW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                        break;
                    }

                    Logger.Log(string.Format("RegistryInteropWrapper.RegEnumKeyExW returns iResult={0}  \n" +
                        "out keyname={1}", iResult.ToString(), pNameBuf.ToString()), Logger.RegistryViewerLoglevel);

                    /*Logger.Log(string.Format("RegistryInteropWrapper.RegEnumKeyEx returns iResult={0}  \n" +
                                        "out keyname={1},\nref NameSize={2},\n ref classname={3},\n" +
                                        "ref pcClass={4},\n out lastWriteTime=highDate:{5},lowDate:{6}", iResult.ToString(), keyname, pcName.ToString(),
                                        classname != null ? classname : "", pcClass.ToString(), lastWriteTime.dwHighDateTime,
                                        lastWriteTime.dwLowDateTime), Logger.RegistryViewerLoglevel);*/

                    KeyInfo.maxKeyLength = 0;
                    KeyInfo.nameSize = pcName;
                    KeyInfo.sKeyname = pNameBuf.ToString().Substring(pNameBuf.ToString().IndexOf(@"\") + 1);
                    KeyInfo.OrigKey = pNameBuf.ToString();
                    KeyInfo.sClassName = pClassBuf.ToString();
                    KeyInfo.filetime = lastWriteTime;

                    enumKeys.Add(KeyInfo);
                    idx++;
                    pNameBuf = new StringBuilder(MAX_KEY_LENGTH);
                    pcName = (uint)MAX_KEY_LENGTH;
                }
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegEnumKeyExW", ex);
            }
            return iResult;
        }

        public static int ApiRegEnumValues(
                               IntPtr handle_t,
                               IntPtr pParentKey,
                               ulong dwValueCount,
                               out List<RegistryValueInfo> enumValues)
        {
            uint idx = 0;
            int iResult = 0;
            enumValues = new List<RegistryValueInfo>();

            while (idx < dwValueCount)
            {
                uint dwValueLen = (uint)MAX_VALUE_LENGTH;
                uint dwValueNameLen = (uint)MAX_KEY_LENGTH;
                byte[] pData = new byte[MAX_VALUE_LENGTH];
                ulong dwDataType = RegistryApi.REG_UNKNOWN;

                StringBuilder pValueNameBuf = new StringBuilder(MAX_KEY_LENGTH);
                RegistryValueInfo valueInfo = new RegistryValueInfo();

                valueInfo.initializeToNull();

                Logger.Log(string.Format("RegistryInteropWrapper.RegEnumValueW(hRegConnection={0}, pParentKey={1}) is called ", handle_t.ToInt32().ToString(), pParentKey.ToInt32().ToString()));

                iResult = RegistryInterop.LwRegEnumValueW(
                                    handle_t,
                                    pParentKey,
                                    idx,
                                    pValueNameBuf,
                                    ref dwValueNameLen,
                                    0,
                                    out dwDataType,
                                    pData,
                                    ref dwValueLen);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegEnumValueW is returns ret={0}", iResult));
                    if (idx++ < dwValueCount)
                        break;
                }

                Logger.Log(string.Format("RegistryInteropWrapper.RegEnumValueW returns iResult={0},\n" +
                                        "ref valuename={1},\nref=dwValueNameLen={2},\nout dType={3},\n" +
                                        "out pData={4},\nref pcData={5}", iResult.ToString(), pValueNameBuf.ToString(), dwValueNameLen.ToString(),
                                        dwDataType.ToString(), (pData != null) ? pData.ToString() : "null", dwValueLen.ToString()), Logger.RegistryViewerLoglevel);

                valueInfo.maxKeyLength = 0;
                valueInfo.pcchValueName = dwValueNameLen;
                valueInfo.pValueName = pValueNameBuf.ToString().Substring(pValueNameBuf.ToString().LastIndexOf(@"\") + 1);
                valueInfo.pcData = dwValueLen;
                valueInfo.pType = (ulong)dwDataType;
                ParseRegistryData(valueInfo, pData);

                if (valueInfo.pValueName.EndsWith("@"))
                    valueInfo.bIsDefault = true;

                enumValues.Add(valueInfo);
                idx++;
            }

            return iResult;
        }

        public static int ApiRegQueryInfoEx(
                            IntPtr hRegConnection,
                            IntPtr hKey,
                            out uint dwSubKeyCount,
                            out uint dwValueCount)
        {
            int iResult = 0;
            string pClass = string.Empty;
            uint dwMaxSubKeyLen = 0;
            ulong pcClass = 0, lpcbMaxClassLen;
            uint lpcbMaxValueLen, dwMaxValueNameLen = 0;
            IntPtr lpcbSecurityDescriptor = IntPtr.Zero;
            RegistryApi.FILETIME filetime;

            dwSubKeyCount = 0;
            dwValueCount = 0;

            try
            {
                Logger.Log(string.Format("RegistryInteropWrapper.RegQueryInfoExW(hRegConnection={0}, hKey={1}) is called ",
                                         hRegConnection.ToInt32().ToString(), hKey.ToInt32().ToString()), Logger.RegistryViewerLoglevel);

                iResult = RegistryInterop.LwRegQueryInfoKeyW(hRegConnection,
                                                hKey,
                                                out pClass,
                                                ref pcClass,
                                                0,
                                                out dwSubKeyCount,
                                                out dwMaxSubKeyLen,
                                                out lpcbMaxClassLen,
                                                out dwValueCount,
                                                out dwMaxValueNameLen,
                                                out lpcbMaxValueLen,
                                                out lpcbSecurityDescriptor,
                                                out filetime);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegQueryInfoExW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.LwRegQueryInfoKeyW() is returns iResult={0}, \n" +
                                "out pClass={1},\n ref pcClass={2},\n" +
                                "out dwSubKeyCount={3},\n out dwMaxSubKeyLen={4}" +
                                "out lpcbMaxClassLen={5},\n out dwValueCount={6},\n" +
                                "out dwMaxValueNameLen={7},\n out lpcbMaxValueLen={8},\n" +
                                "out lpcbMaxValueLen={9},\n out filetime={10}:{11}", iResult.ToString(), pClass, pcClass.ToString(),
                                dwSubKeyCount.ToString(), dwMaxSubKeyLen.ToString(), lpcbMaxClassLen.ToString(),
                                dwValueCount.ToString(), dwMaxValueNameLen.ToString(), lpcbMaxValueLen.ToString(),
                                lpcbMaxValueLen.ToString(), filetime.dwHighDateTime.ToString(), filetime.dwLowDateTime.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegQueryInfoExW", ex);
            }

            return iResult;
        }

        public static int ApiRegCreateKeyEx(
                            IntPtr hRegConnection,
                            IntPtr hKey,
                            IntPtr pSubkey,
                            out IntPtr phkResult)
        {
            int iResult = 0;
            ulong pdwDisposition = 0;
            RegistryApi.SECURITY_ATTRIBUTES pSecurityAttributes = new RegistryApi.SECURITY_ATTRIBUTES();
            phkResult = IntPtr.Zero;

            try
            {
                Logger.Log(string.Format("RegistryInteropWrapper.RegCreateKeyExW(hRegConnection={0}, hKey={1} sSubkey={2}),  is called ",
                          hRegConnection.ToInt32().ToString(), hKey.ToInt32().ToString(), Marshal.PtrToStringUni(pSubkey)), Logger.RegistryViewerLoglevel);

                if (pSubkey == IntPtr.Zero)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegCreateKeyExW has invalid parameter"));
                    return iResult;
                }

                iResult = RegistryInterop.LwRegCreateKeyExW(hRegConnection,
                                                hKey,
                                                pSubkey,
                                                0,
                                                IntPtr.Zero,
                                                0,
                                                RegistryApi.RegSAM.CreateSubKey,
                                                ref pSecurityAttributes,
                                                out phkResult,
                                                out pdwDisposition);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegCreateKeyExW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.LwRegCreateKeyExW() is returns iResult={0}, \n" +
                                "out phkResult={1}, out pdwDisposition={1}", iResult.ToString(), phkResult.ToInt32().ToString(), pdwDisposition.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegCreateKeyExW", ex);
            }

            return iResult;
        }


        public static int ApiRegSetValueEx(
                            IntPtr hRegConnection,
                            IntPtr hKey,
                            string sValuename,
                            uint dwType,
                            byte[] pData)
        {
            int iResult = 0;

            try
            {
                uint pcData = (uint)pData.Length;
                StringBuilder pValueNameBuf = new StringBuilder();
                pValueNameBuf.Append(sValuename.Trim());

                Logger.Log(string.Format("RegistryInteropWrapper.RegSetValueExW(hRegConnection={0}, hKey={1} sValuename={2}),  is called ", hRegConnection.ToInt32().ToString(), hKey.ToInt32().ToString(), String.IsNullOrEmpty(sValuename) ? "" : sValuename), Logger.RegistryViewerLoglevel);

                iResult = RegistryInterop.LwRegSetValueExW(hRegConnection,
                                                hKey,
                                                sValuename.Trim(),
                                                0,
                                                dwType,
                                                pData,
                                                pcData);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegSetValueExW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.LwRegSetValueExW() is returns iResult={0}, \n" +
                                "ref pcData={1}", iResult.ToString(), pcData.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegSetValueExW", ex);
            }

            return iResult;
        }

        public static int ApiRegSetKeyValue(
                           IntPtr hRegConnection,
                           IntPtr hKey,
                           string skeyname,
                           string sValuename,
                           int dwType,
                           string pData)
        {
            int iResult = 0;
            uint pcData = (uint)pData.Length;

            try
            {
                StringBuilder sbKeyname = new StringBuilder(MAX_KEY_LENGTH);
                sbKeyname.Append(skeyname);

                StringBuilder sbValuename = new StringBuilder(MAX_VALUE_LENGTH);
                sbValuename.Append(sValuename);

                Logger.Log(string.Format("RegistryInteropWrapper.RegSetValueExW(hRegConnection={0}, hKey={1} sValuename={2}),  is called ", hRegConnection.ToInt32().ToString(), hKey.ToInt32().ToString(), sValuename), Logger.RegistryViewerLoglevel);

                iResult = RegistryInterop.LwRegSetKeyValueW(hRegConnection,
                                                hKey,
                                                sbKeyname,
                                                sbValuename,
                                                dwType,
                                                pData,
                                                pcData);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegSetValueExW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.LwRegSetValueExW() is returns iResult={0}, \n" +
                                "ref pcData={1}", iResult.ToString(), pcData.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegSetValueExW", ex);
            }

            return iResult;
        }

        public static int ApiRegGetValue(IntPtr hRegConnection,
                                         RegistryValueInfo valueInfo,
                                         out object pbData)
        {
            pbData = null;
            int iResult = -1;
            uint dwValueLen = (uint)MAX_VALUE_LENGTH;
            byte[] pData = new byte[MAX_VALUE_LENGTH];
            ulong dwType = RegistryApi.REG_UNKNOWN;

            try
            {
                Logger.Log(string.Format("RegistryInteropWrapper.RegGetValueW(hRegConnection={0}, hKey={1}, sValue={2}),  is called ",
                    hRegConnection.ToInt32().ToString(), valueInfo.pParentKey.ToInt32().ToString(), valueInfo.pValueName), Logger.RegistryViewerLoglevel);

                iResult = RegistryInterop.LwRegGetValueW(hRegConnection,
                                                valueInfo.pParentKey,
                                                null,
                                                valueInfo.pValueName,
                                                GetTypeFlags(valueInfo.pType),
                                                out dwType,
                                                pData,
                                                ref dwValueLen);
                //iResult = ApiRegQueryValueEx(hRegConnection, valueInfo, out pbData);

                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegGetValueW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                    return iResult;
                }

                Array.Resize<byte>(ref pData, (int)dwValueLen);

                //Since for the default value the dwValueLen is 1024 even if the value not set
                if (dwType == RegistryApi.REG_SZ)
                {
                    List<byte> bytList = new List<byte>();
                    foreach (byte byt in pData)
                        if (byt != 0)
                            bytList.Add(byt);
                    if (bytList.Count != 0)
                    {
                        pData = new byte[bytList.Count];
                        bytList.CopyTo(pData);
                    }
                    else
                        pData = null;
                }
                pbData = pData;

                Logger.Log(string.Format("RegistryInterop.LwRegGetValueW() is returns iResult={0}, \n" +
                                "ref dwValueLen={1}", iResult.ToString(), dwValueLen.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegGetValueW", ex);
            }

            return iResult;
        }

        public static int ApiRegQueryMultipleValues(IntPtr hRegConnection,
                                                    IntPtr hKey,
                                                    uint dwValueNum,
                                                    out object pbData)
        {
            int iResult = 0;
            uint dwTotalSize = (uint)(MAX_VALUE_LENGTH * dwValueNum);
            RegistryValueInfo[] valueList = null;
            RegistryApi.VALENT Val_list = new RegistryApi.VALENT();
            IntPtr pVal_list = Marshal.AllocHGlobal(Marshal.SizeOf(typeof(RegistryApi.VALENT)));
            StringBuilder pValueNameBuf = new StringBuilder(MAX_KEY_LENGTH);

            try
            {
                Logger.Log(string.Format("RegistryInteropWrapper.RegQueryMultipleValuesW(hRegConnection={0}, hKey={1}),  is called ",
                    hRegConnection.ToInt32().ToString(), hKey.ToInt32().ToString()), Logger.RegistryViewerLoglevel);

                iResult = RegistryInterop.LwRegQueryMultipleValuesW(
                                                hRegConnection,
                                                hKey,
                                                pVal_list,
                                                dwValueNum,
                                                pValueNameBuf,
                                                ref dwTotalSize);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegQueryMultipleValuesW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                }

                if (pVal_list != IntPtr.Zero)
                {
                    IntPtr pCur = pVal_list;
                    valueList = new RegistryValueInfo[dwValueNum];
                    for (int idx = 0; idx < dwValueNum; idx++)
                    {
                        Marshal.PtrToStructure(pCur, Val_list);
                        RegistryValueInfo valueInfo = new RegistryValueInfo();
                        valueInfo.pParentKey = hKey;
                        valueInfo.pValueName = Val_list.ve_valuename.ToString(); ;
                        valueInfo.bDataBuf = Val_list.ve_valueptr;
                        valueList[idx] = valueInfo;

                        pCur = (IntPtr)((int)pCur + Marshal.SizeOf(Val_list));
                    }
                }

                Logger.Log(string.Format("RegistryInterop.LwRegQueryMultipleValuesW() is returns iResult={0}",
                                iResult.ToString()), Logger.RegistryViewerLoglevel);

                pbData = valueList;
            }
            catch (Exception ex)
            {
                pbData = null;
                Logger.LogException("RegistryInteropWrapper.RegQueryMultipleValuesW", ex);
            }

            return iResult;
        }

        public static int ApiRegQueryValueEx(IntPtr hRegConnection,
                                             RegistryValueInfo valueInfo,
                                             out object pbData)
        {
            pbData = null;
            int iResult = 0;
            uint dwValueLen = (uint)MAX_VALUE_LENGTH;
            byte[] pData = new byte[MAX_VALUE_LENGTH];
            ulong dwType = RegistryApi.REG_SZ;

            try
            {
                Logger.Log(string.Format("RegistryInteropWrapper.ApiRegQueryValueExW(hRegConnection={0}, hKey={1}),  is called ",
                    hRegConnection.ToInt32().ToString(), valueInfo.pParentKey.ToInt32().ToString()), Logger.RegistryViewerLoglevel);

                iResult = RegistryInterop.LwRegQueryValueExW(hRegConnection,
                                    valueInfo.pParentKey,
                                    valueInfo.pValueName,
                                    0,
                                    out dwType,
                                    out pData,
                                    out dwValueLen);

                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.ApiRegQueryValueExW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.ApiRegQueryValueExW() is returns iResult={0}",
                                iResult.ToString()), Logger.RegistryViewerLoglevel);

                valueInfo.pcData = dwValueLen;
                valueInfo.pType = (ulong)dwType;

                Array.Resize<byte>(ref pData, (int)dwValueLen);


                pbData = pData;
            }
            catch (Exception ex)
            {
                pbData = pData;
                Logger.LogException("RegistryInteropWrapper.ApiRegQueryValueExW", ex);
            }
            return iResult;
        }

        public static int ApiRegDeleteTree(
                           IntPtr hRegConnection,
                           IntPtr pParentKey,
                           IntPtr hKey,
                           string sSubkey)
        {
            int iResult = 0;

            try
            {
                Logger.Log(string.Format("RegistryInteropWrapper.RegDeleteTreeW(hRegConnection={0}, pParentKey={1}, hKey={2} sSubkey={3}),  is called ",
                    hRegConnection.ToInt32().ToString(), pParentKey.ToInt32().ToString(), hKey.ToInt32().ToString(), sSubkey), Logger.RegistryViewerLoglevel);

                if (ApiRegCloseKey(hRegConnection, hKey) == 0)
                {
                    iResult = RegistryInterop.LwRegDeleteTreeW(hRegConnection,
                                                    pParentKey,
                                                    sSubkey);
                }

                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegDeleteTreeW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.LwRegDeleteTreeW() is returns iResult={0}",
                                iResult.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegDeleteTreeW", ex);
            }

            return iResult;
        }

        public static int ApiRegDeleteKey(
                          IntPtr hRegConnection,
                          IntPtr pParentKey,
                          IntPtr hKey,
                          string sSubkey)
        {
            int iResult = 0;

            try
            {
                Logger.Log(string.Format("RegistryInteropWrapper.RegDeleteKeyW(hRegConnection={0}, pParentKey= {1}, hKey={2} sSubkey={3}),  is called ",
                    hRegConnection.ToInt32().ToString(), pParentKey.ToInt32().ToString(), hKey.ToInt32().ToString(), sSubkey != null ? sSubkey : ""), Logger.RegistryViewerLoglevel);

                if (ApiRegCloseKey(hRegConnection, hKey) == 0)
                {
                    iResult = RegistryInterop.LwRegDeleteKeyW(hRegConnection,
                                                    pParentKey,
                                                    sSubkey);
                }

                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegDeleteKeyW is returns ret={0}", iResult.ToString()), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.LwRegDeleteKeyW() is returns iResult={0}\n",
                                iResult.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegDeleteKeyW", ex);
            }

            return iResult;
        }

        public static int ApiRegCloseKey(
                         IntPtr hRegConnection,
                         IntPtr hKey)
        {
            int iResult = 0;

            try
            {
                Logger.Log(string.Format("RegistryInteropWrapper.ApiRegCloseKey(hRegConnection={0}, hKey={1} is called ",
                    hRegConnection.ToInt32().ToString(), hKey.ToInt32().ToString()), Logger.RegistryViewerLoglevel);

                iResult = RegistryInterop.LwRegCloseKey(hRegConnection,
                                                hKey);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.ApiRegCloseKey is returns ret={0}", iResult.ToString()), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.ApiRegCloseKey() is returns iResult={0}\n",
                                iResult.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.ApiRegCloseKey", ex);
            }

            return iResult;
        }


        public static int ApiRegDeleteKeyValue(
                          IntPtr hRegConnection,
                          IntPtr hKey,
                          string sSubkey,
                          string sValuename)
        {
            int iResult = 0;

            try
            {
                StringBuilder sbKeyname = new StringBuilder(MAX_KEY_LENGTH);
                sbKeyname.Append(sSubkey);

                Logger.Log(string.Format("RegistryInteropWrapper.RegDeleteKeyValueW(hRegConnection={0}, hKey={1} sSubkey={2}),  is called ",
                    hRegConnection.ToInt32().ToString(), hKey.ToInt32().ToString(), sSubkey != null ? sSubkey : ""), Logger.RegistryViewerLoglevel);

                iResult = RegistryInterop.LwRegDeleteKeyValueW(hRegConnection,
                                                hKey,
                                                sbKeyname,
                                                sValuename);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.RegDeleteKeyValueW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.LwRegDeleteKeyValueW() is returns iResult={0}",
                                iResult.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegDeleteKeyValueW", ex);
            }

            return iResult;
        }

        public static int ApiRegDeleteValue(
                        IntPtr hRegConnection,
                        IntPtr hKey,
                        string sValuename)
        {
            int iResult = 0;

            try
                {
                Logger.Log(string.Format("RegistryInteropWrapper.ApiRegDeleteValueW(hRegConnection={0}, hKey={1} sValuename={2}),  is called ",
                    hRegConnection.ToInt32().ToString(), hKey.ToInt32().ToString(), sValuename != null ? sValuename : ""), Logger.RegistryViewerLoglevel);

                iResult = RegistryInterop.LwRegDeleteValueW(hRegConnection,
                                                hKey,
                                                sValuename);
                if (iResult != 0)
                {
                    Logger.Log(string.Format("RegistryInteropWrapper.ApiRegDeleteValueW is returns ret={0}", iResult), Logger.RegistryViewerLoglevel);
                }

                Logger.Log(string.Format("RegistryInterop.ApiRegDeleteValueW() is returns iResult={0}",
                                iResult.ToString()), Logger.RegistryViewerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.ApiRegDeleteValueW", ex);
            }

            return iResult;
        }

		public static int GetRegGetValueW(IntPtr  hRegConnection,
		                             RegistryValueInfo valueinfo)
		{
			int iResult = -1;

            try
                {
	                Logger.Log(string.Format("RegistryInteropWrapper.GetRegGetValueW(hRegConnection={0}, skeyname={1}, sValuename={2}),  is called ",
	                    hRegConnection.ToInt32().ToString(), valueinfo.sKeyname, valueinfo.pValueName), Logger.RegistryViewerLoglevel);

					uint dwValueLen = (uint)MAX_VALUE_LENGTH;
					byte[] pData = new byte[MAX_VALUE_LENGTH];
					ulong dwType = RegistryApi.REG_UNKNOWN;

					RegistryInterop.LwRegGetValueW(hRegConnection,
		                             valueinfo.pParentKey,
						 null,
						 valueinfo.pValueName,
						 GetTypeFlags(valueinfo.pType),
						 out dwType,
						 pData,
						 ref dwValueLen);

				valueinfo.bDataBuf = pData;
			}
			catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.GetRegGetValueW", ex);
            }

            return iResult;
		}



        #endregion

        #region Helper functions


        public static void RegModifyKeyValue(RegistryValueInfo ValueInfo, out Byte[] pData)
        {
            pData = null;
            UnicodeEncoding encoder = new UnicodeEncoding();

            try
            {
                switch (ValueInfo.pType)
                {
                    case (ulong)RegistryApi.REG_SZ:
                    case (ulong)RegistryApi.REG_PLAIN_TEXT:
                    case (ulong)RegistryApi.REG_EXPAND_SZ:
                        pData = encoder.GetBytes(ValueInfo.bDataBuf.ToString().Trim().ToCharArray(),
                                                 0, ValueInfo.bDataBuf.ToString().Trim().Length);
                        if (pData.Length < MAX_VALUE_LENGTH)
                            Array.Resize<byte>(ref pData, pData.Length + 1);
                        else
                            pData[MAX_VALUE_LENGTH - 1] = 0;

                        break;

                    case (ulong)RegistryApi.REG_MULTI_SZ:
                        string[] sDataArry = ValueInfo.bDataBuf as string[];
                        if (sDataArry != null)
                        {
                            StringBuilder sBuilder = new StringBuilder();
                            foreach (string tempString in sDataArry)
                            {
                                if (!string.IsNullOrEmpty(tempString))
								{
                                    sBuilder = sBuilder.Append(tempString);
									sBuilder = sBuilder.Append('\0');
								}
                            }
							sBuilder = sBuilder.Append('\0');
							sBuilder = sBuilder.Append('\0');
                            pData = encoder.GetBytes(sBuilder.ToString());
                        }
                        else
                            pData = ValueInfo.bDataBuf as byte[];
                        break;

                    case (ulong)RegistryApi.REG_DWORD:
                        uint uTemp = (uint)(int)ValueInfo.bDataBuf;
                        byte[] dwDataArray = BitConverter.GetBytes(UInt32.Parse(uTemp.ToString()));
                        pData = dwDataArray;
                        break;

                    case (ulong)RegistryApi.REG_QWORD:
						ulong dwTemp = (ulong)(long)ValueInfo.bDataBuf;
                        byte[] qwDataArry = BitConverter.GetBytes(UInt64.Parse(dwTemp.ToString()));
                        pData = qwDataArry;
                        break;

                    case (ulong)RegistryApi.REG_BINARY:
                    case (ulong)RegistryApi.REG_FULL_RESOURCE_DESCRIPTOR:
                    case (ulong)RegistryApi.REG_RESOURCE_LIST:
                    case (ulong)RegistryApi.REG_RESOURCE_REQUIREMENTS_LIST:
                    case (ulong)RegistryApi.REG_NONE:
                        byte[] DataArry = ValueInfo.bDataBuf as byte[];
                        pData = DataArry;
                        break;

                    default:
                        break;
                }
                if (pData != null && pData.Length > MAX_VALUE_LENGTH)
                    Array.Resize<byte>(ref pData, 1024);
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegModifyKeyValue", ex);
            }
        }

        public static int ParseRegistryData(RegistryValueInfo ValueInfo, byte[] pData)
        {
            int iResult = 0;
            Array.Resize<byte>(ref pData, (int)ValueInfo.pcData);
            try
            {
                StringBuilder sbTemp = new StringBuilder();
                switch (ValueInfo.pType)
                {
                    case (ulong)RegistryApi.REG_SZ:
                    case (ulong)RegistryApi.REG_PLAIN_TEXT:
                    case (ulong)RegistryApi.REG_EXPAND_SZ:
                        ValueInfo.bDataBuf = new UnicodeEncoding().GetString(pData).ToString().Split(new char[] { '\0' })[0];
                        break;

                    case (ulong)RegistryApi.REG_MULTI_SZ:
                        string[] sTempArry = new UnicodeEncoding().GetString(pData).Split('\0');
                        foreach (string sValue in sTempArry)
                        {
                            sbTemp.Append(sValue + " ");
                        }
                        ValueInfo.bDataBuf = sbTemp.ToString();
                        break;

                    case (ulong)RegistryApi.REG_DWORD:
                        string sDTemp = BitConverter.ToUInt32(pData, 0).ToString();
                        ValueInfo.bDataBuf = string.Concat("0x" + RegistryUtils.DecimalToBase(BitConverter.ToUInt32(pData, 0), 16).PadLeft(8, '0'), "(" + sDTemp + ")");
                        break;

                    case (ulong)RegistryApi.REG_QWORD:
                        string sQTemp = BitConverter.ToInt64(pData, 0).ToString();
                        ValueInfo.bDataBuf = string.Concat("0x" + RegistryUtils.DecimalToBase((uint)BitConverter.ToInt64(pData, 0), 16).PadLeft(16, '0'), "(", sQTemp + ")");
                        break;

                    case (ulong)RegistryApi.REG_BINARY:
                    case (ulong)RegistryApi.REG_FULL_RESOURCE_DESCRIPTOR:
                    case (ulong)RegistryApi.REG_RESOURCE_LIST:
                    case (ulong)RegistryApi.REG_RESOURCE_REQUIREMENTS_LIST:
                    case (ulong)RegistryApi.REG_NONE:
                        ValueInfo.bDataBuf = RegParseValueKind(pData, ValueInfo);
                        break;

                    default:
                        break;
                }
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.ParseRegistryData", ex);
            }

            return iResult;
        }

        public static object RegParseValueKind(byte[] BinaryData, RegistryValueInfo valueInfo)
        {
            object sDataBuf = null;
            StringBuilder sbTemp = new StringBuilder();

            try
            {
                switch (valueInfo.pType)
                {
                    case (ulong)RegistryApi.REG_BINARY:
                        if (BinaryData != null && BinaryData.Length != 0)
                        {
                            for (ulong idx = 0; idx < valueInfo.pcData; idx++)
                            {
                                string stringValue = BitConverter.ToString(new byte[] { BinaryData[idx] });
                                if (stringValue.Length == 1)
                                    stringValue = "0" + stringValue;
                                sbTemp.Append(stringValue);
                                sbTemp.Append(" ");
                            }
                            sDataBuf = sbTemp.ToString();
                        }
                        break;

                    case (ulong)RegistryApi.REG_EXPAND_SZ:
                        if (BinaryData != null && BinaryData.Length != 0)
                        {
                            string stringValue = string.Empty;
                            for (ulong idx = 0; idx < valueInfo.pcData; idx++)
                                stringValue = stringValue + BitConverter.ToString(new byte[] { BinaryData[idx] });

                            sDataBuf = stringValue;
                        }
                        break;

                    default:
                        sDataBuf = BinaryData;
                        break;
                }
            }
            catch (Exception ex)
            {
                Logger.LogException("RegistryInteropWrapper.RegParseValueKind", ex);
            }

            return sDataBuf;
        }

        public static byte[] RegParseBinaryData(IntPtr pData, ulong pDataLen)
        {
            int size = Marshal.SizeOf(pData);
            byte[] binaryValue = new byte[pDataLen];

            for (ulong i = 0; i < pDataLen; i++)
                binaryValue[i] = Marshal.ReadByte(pData, (int)i);

            return binaryValue;
        }

        public static IntPtr GethKeyPTR(long keyType)
        {
            IntPtr pHkey = Marshal.AllocHGlobal(sizeof(long));
            Marshal.WriteInt64(pHkey, keyType);

            return pHkey;
        }

        public static UIntPtr GethKeyUPTR(long keyType)
        {
            UIntPtr upHkey;

            unsafe
            {
                IntPtr pHkey = Marshal.AllocHGlobal(sizeof(long));
                Marshal.WriteInt64(pHkey, keyType);

                upHkey = (UIntPtr)pHkey.ToPointer();
            }
            return upHkey;
        }

        private static byte[] iConvToMultibyteArray(String multibyteString)
        {
            byte[] result = ASCIIEncoding.ASCII.GetBytes(multibyteString);
            return result;
        }

        public static String ConvToUnicode(int codepage, String multibyteString)
        {
            byte[] b = (byte[])iConvToMultibyteArray(multibyteString);
            return (String)ToUnicode((uint)(int)codepage, b);
        }

        private static string ToUnicode(uint codepage, Byte[] lpMultiByteStr)
        {
            Byte[] lpWideCharStr = new Byte[2 * lpMultiByteStr.Length];
            RegistryInterop.MultiByteToWideChar(codepage, 0, lpMultiByteStr, lpMultiByteStr.Length,
               lpWideCharStr, 2 * lpMultiByteStr.Length);
            return Encoding.Unicode.GetString(lpWideCharStr);
        }

        public static uint GetTypeFlags(ulong pType)
        {
            uint dwFlags = RegistryApi.RRF_RT_REG_NONE;

            switch (pType)
            {
                case (ulong)RegistryApi.REG_BINARY:
                    dwFlags = RegistryApi.RRF_RT_REG_BINARY;
                    break;

                case (ulong)RegistryApi.REG_DWORD:
                    dwFlags = RegistryApi.RRF_RT_DWORD;
                    break;

                case (ulong)RegistryApi.REG_EXPAND_SZ:
                    dwFlags = RegistryApi.RRF_RT_REG_EXPAND_SZ;
                    break;

                case (ulong)RegistryApi.REG_SZ:
                case (ulong)RegistryApi.REG_PLAIN_TEXT:
                    dwFlags = RegistryApi.RRF_RT_REG_SZ;
                    break;

                case (ulong)RegistryApi.REG_MULTI_SZ:
                    dwFlags = RegistryApi.RRF_RT_REG_MULTI_SZ;
                    break;

                case (ulong)RegistryApi.REG_QWORD:
                    dwFlags = RegistryApi.RRF_RT_REG_QWORD;
                    break;

                default:
                    dwFlags = RegistryApi.RRF_RT_REG_NONE;
                    break;
            }
            return dwFlags;
        }


        #endregion

    }
}
