using System;
using System.Security.Cryptography;
using System.Text;
using System.Management;

namespace coolapp.Classes
{
class Identification
{
public static string GetHardwareID()
{
string toHash = Identifier("Win32_Processor", "ProcessorId");
toHash += "-" + Identifier("Win32_BIOS", "SerialNumber");
toHash += "-" + Identifier("Win32_DiskDrive", "SerialNumber");
toHash += "-" + Identifier("Win32_BaseBoard", "SerialNumber");
toHash += "-" + Identifier("Win32_VideoController", "Name");
return Hash(toHash);
}
    private static string Identifier(string wmiClass, string wmiProperty)
    {
        string result = "";
        using (ManagementClass mc = new ManagementClass(wmiClass))
        {
            foreach (ManagementObject mo in mc.GetInstances())
            {
                if (result == "")
                {
                    try
                    {
                        result = mo[wmiProperty].ToString();
                        break;
                    }
                    catch { }
                }
            }
        }
        return result;
    }

    private static string Hash(string input)
    {
        using (var sha256 = SHA256.Create())
        {
            var bytes = Encoding.UTF8.GetBytes(input);
            var hash = sha256.ComputeHash(bytes);
            return Convert.ToBase64String(hash);
        }
    }

    public static string OsName()
    {
        return $"{new Microsoft.VisualBasic.Devices.ComputerInfo().OSFullName.Replace("Microsoft ", "")} {Environment.GetEnvironmentVariable("PROCESSOR_ARCHITECTURE")}";
    }
}
    }
