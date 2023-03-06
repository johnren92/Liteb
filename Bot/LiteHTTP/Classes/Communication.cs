using System;
using System.IO;
using System.Net;
using System.Security.Cryptography;
using System.Text;

namespace LiteHTTP.Classes
{
    class Communication
    {
        public static string makeRequest(string url, string parameters)
        {
            try
            {
                string result = null;
                byte[] param = Encoding.UTF8.GetBytes(parameters);
                HttpWebRequest req = (HttpWebRequest)WebRequest.Create(url);
                req.Method = "POST";
                req.UserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.36 Edge/16.16299";
                req.ContentType = "application/x-www-form-urlencoded";
                req.ContentLength = param.Length;
                req.Accept = "*/*";
                req.KeepAlive = true;
                req.ServicePoint.Expect100Continue = false;
                using (Stream st = req.GetRequestStream())
                {
                    st.Write(param, 0, param.Length);
                }
                using (WebResponse resp = req.GetResponse())
                {
                    using (StreamReader sr = new StreamReader(resp.GetResponseStream()))
                    {
                        result = sr.ReadToEnd();
                    }
                }
                return result;
            }
            catch
            {
                return "rqf";
            }
        }

        public static string encrypt(string input)
        {
            try
            {
                string key = Settings.edkey;
                RijndaelManaged rj = new RijndaelManaged();
                rj.Padding = PaddingMode.PKCS7;
                rj.Mode = CipherMode.CBC;
                rj.KeySize = 256;
                rj.BlockSize = 128;
                byte[] ky = Encoding.ASCII.GetBytes(key);
                byte[] inp = Encoding.UTF8.GetBytes(input);
                byte[] res;
                using (ICryptoTransform enc = rj.CreateEncryptor(ky, ky))
                {
                    using (MemoryStream ms = new MemoryStream())
                    {
                        using (CryptoStream cs = new CryptoStream(ms, enc, CryptoStreamMode.Write))
                        {
                            cs.Write(inp, 0, inp.Length);
                        }
                        res = ms.ToArray();
                    }
                }
                return Convert.ToBase64String(res).Replace("+", "-").Replace("/", "_");
            }
            catch
            {
                return null;
            }
        }

        public static string decrypt(string input)
        {
            try
            {
                string key = Settings.edkey;
                RijndaelManaged rj = new RijndaelManaged();
                rj.Padding = PaddingMode.PKCS7;
                rj.Mode = CipherMode.CBC;
                rj.KeySize = 256;
                rj.BlockSize = 128;
                byte[] ky = Encoding.ASCII.GetBytes(key);
                byte[] inp = Convert.FromBase64String(input.Replace("-", "+").Replace("_", "/"));
                byte[] res;
                using (ICryptoTransform dec = rj.CreateDecryptor(ky, ky))
                {
                    using (MemoryStream ms = new MemoryStream(inp))
                    {
                        using (CryptoStream cs = new CryptoStream(ms, dec, CryptoStreamMode.Read))
                        {
                            byte[] buffer = new byte[inp.Length];
                            int bytesRead = cs.Read(buffer, 0, buffer.Length);
                            res = new byte[bytesRead];
                            Array.Copy(buffer, res, bytesRead);
                        }
                    }
                }
                return Encoding.UTF8.GetString(res);
            }
            catch
            {
                return null;
            }
        }
    }
}
