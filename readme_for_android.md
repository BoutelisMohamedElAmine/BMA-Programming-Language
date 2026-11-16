# 📱 دليل استخدام لغة BMA على أندرويد (Termux)

مرحباً بك! هذا الدليل مخصص للمستخدمين الجدد الذين يريدون تجربة لغة **BMA** على هواتفهم الذكية باستخدام تطبيق **Termux**، دون الحاجة إلى أي معرفة مسبقة بالبرمجة أو أدوات التطوير.

---

## 1. تثبيت Termux
Termux هو تطبيق طرفية (Terminal) يعمل على أندرويد.

*   **هام جداً:** قم بتنزيل Termux من متجر **F-Droid** (وليس من Google Play)، لأن إصدار Google Play قديم ولا يعمل بشكل صحيح مع الأدوات الحديثة.
*   رابط التحميل: [F-Droid - Termux](https://f-droid.org/repo/com.termux_118.apk)

---

## 2. تنزيل مترجم BMA (الملف التنفيذي)
بعد تثبيت Termux وفتحه، اكتب الأوامر التالية:

```bash
# تحديث الحزم الأساسية
pkg update && pkg upgrade -y

# تثبيت أداة التنزيل
pkg install wget -y

# تنزيل مترجم BMA الجاهز لأندرويد
wget https://github.com/BoutelisMohamedElAmine/BMA-Programming-Language/raw/main/bma-android.bin

# منح صلاحية التنفيذ للملف
chmod +x bma-android.bin
