#The explanation is provided in Arabic .
================================================================================
                    BMA PROGRAMMING LANGUAGE
              الدستور الرسمي والقوانين الكاملة
================================================================================

الإصدار    : 1.0.0
التاريخ    : 2026
المؤلف     : Boutelis Mohamed El Amine
الاسم      : BMA = Boutelis Mohamed El Amine
المترجم    : BMA_Compiler.exe
الامتدادات : .bma (برامج) | .bmalib (مكتبات)
الترميز    : UTF-8

================================================================================
                              جدول المحتويات
================================================================================

الباب الأول    : التعريف باللغة والفلسفة
الباب الثاني   : البنية العامة للملف
الباب الثالث   : الكلمات المحجوزة
الباب الرابع   : نظام الأنواع
الباب الخامس   : المتغيرات والنطاقات
الباب السادس   : التعبيرات والعوامل
الباب السابع   : التحكم في التدفق
الباب الثامن   : الدوال
الباب التاسع   : المصفوفات
الباب العاشر  : الإدخال والإخراج
الباب الحادي عشر     : المكتبات
الباب الثاني عشر     : FFI (استدعاء DLL)
الباب الثالث عشر     : إدارة الذاكرة
الباب الرابع عشر     : المكتبة القياسية
الباب الخامس عشر     : الأمثلة الكاملة
الباب السادس عشر     : القواعد الذهبية
الباب السابع عشر     : القيود المعروفة
الباب الثامن عشر    : خارطة التطوير
الباب التاسع عشر    : نموذج الذكاء الاصطناعي
الباب العشرون        : الخاتمة والترخيص

================================================================================
                    الباب الأول: التعريف والفلسفة
================================================================================

1.1 ما هي BMA؟
----------------
BMA هي لغة برمجة عالية المستوى، عربية-لاتينية، صُممت لتكون:
  - بسيطة كنحو Python
  - قوية كنظام أنواع Delphi
  - قابلة للتوسع عبر مكتبات خارجية
  - مستقلة عن أي بيئة تطوير بعد بنائها

BMA هي اختصار لاسم مؤسسها:
  B = #Boutelis
  M = #Mohamed
  A = El #Amine

المترجم: BMA_Compiler.exe (مكتوب بلغة Delphi)
طريقة التنفيذ: مفسّر AST (Abstract Syntax Tree Interpreter)

1.2 الفلسفة
------------
  الوضوح فوق الغموض    : كل جملة تنتهي بنقطة .
  الأنواع الصريحة      : كل متغير له نوع ثابت
  التوسع عبر المكتبات  : كل ميزة جديدة تأتي من .bmalib
  الاستقلال عن دلفي    : FFI يسمح باستدعاء أي DLL
  البساطة أولًا        : كل ميزة تُضاف عند الحاجة فقط

1.3 الرؤية
-----------
أن تصبح BMA لغة برمجة كاملة تدعم:
  - تطبيقات سطح المكتب
  - تطبيقات الويب
  - التكامل مع مكتبات Delphi وPython
  - التطوير الذاتي عبر الذكاء الاصطناعي

================================================================================
                    الباب الثاني: البنية العامة
================================================================================

2.1 بنية ملف .bma (الترتيب إلزامي)
------------------------------------
  BMA_Pro : program_name .          <- إلزامي
  library : lib1 , lib2 .           <- اختياري
  extern_lib : 'dll.dll' .          <- اختياري (FFI)
  extern : ...                      <- اختياري (FFI)
  variable : a=integer , b=string . <- اختياري
  function : ... end_func .         <- اختياري
  code :                             <- إلزامي
    ...
  end .

2.2 بنية ملف .bmalib (مكتبة)
------------------------------
  BMA_Lib : library_name .
  extern_lib : 'dll.dll' .
  extern : fname ( ... ) return type from 'Actual' stdcall .
  native : fname ( ... ) return type .
  function : fname ( ... ) return type .
    ...
  end_func .
  end_lib .

2.3 التعليقات
--------------
  // تعليق سطر واحد
  /* تعليق
     متعدد الأسطر */

2.4 الترميز
------------
UTF-8 إلزامي. يُقبل مع أو بدون BOM.

================================================================================
                    الباب الثالث: الكلمات المحجوزة
================================================================================

3.1 بنية البرنامج
------------------
  BMA_Pro      رأس البرنامج
  BMA_Lib      رأس المكتبة
  library      قسم المكتبات
  variable     قسم المتغيرات
  code         كتلة الكود
  end          نهاية الكتلة الرئيسية
  end_if       نهاية الشرط
  end_while    نهاية حلقة while
  end_for      نهاية حلقة for
  end_func     نهاية الدالة
  end_lib      نهاية المكتبة

3.2 التحكم في التدفق
---------------------
  if / then / else
  while / do
  for / to / step
  break / continue / return

3.3 الدوال
----------
  function     تعريف دالة
  native       دالة أصلية في المترجم
  extern       دالة خارجية عبر FFI
  extern_lib   تعريف مكتبة DLL
  from         اسم الدالة الحقيقي في DLL

3.4 اتفاقيات الاستدعاء (FFI)
-----------------------------
  stdcall      (افتراضي - لـ Windows API)
  cdecl        (لـ مكتبات C)

3.5 المنطق
----------
  and / or / not / true / false

3.6 الإدخال والإخراج
--------------------
  write        طباعة
  read         قراءة

3.7 الأنواع
-----------
  integer / real / string / boolean / void
  array_integer / array_real / array_string / array_boolean
  pointer / wstring / int64 / real32

3.8 العوامل
-----------
  mod / div

ملاحظة: كل الكلمات lowercase وحساسة لحالة الأحرف.

================================================================================
                    الباب الرابع: نظام الأنواع
================================================================================

4.1 الأنواع الأساسية
---------------------
  النوع      | القيم              | الحجم    | القيمة الافتراضية
  -----------|--------------------|---------|-------------------
  integer    | أعداد صحيحة 64-bit | 8 بايت  | 0
  real       | أعداد عشرية 64-bit | 8 بايت  | 0.0
  string     | نصوص UTF-16        | مرجع    | ''
  boolean    | true / false       | 1 بايت  | false
  void       | لا قيمة (للدوال)   | -       | -

4.2 أنواع المصفوفات
-------------------
  array_integer   مصفوفة أعداد صحيحة
  array_real      مصفوفة أعداد عشرية
  array_string    مصفوفة نصوص
  array_boolean   مصفوفة قيم منطقية

4.3 أنواع FFI (للاستخدام مع extern فقط)
----------------------------------------
  pointer   مؤشر خام (void*)
  wstring   نص Unicode (PWideChar)
  int64     عدد 64-bit (مخطط)
  real32    عدد عشري 32-bit (مخطط)

4.4 التحويلات
-------------
  integer -> real    تلقائي وآمن
  real -> integer    ممنوع (استخدم floor/ceil/round)
  integer -> string  ممنوع (استخدم to_string)
  string -> integer  ممنوع (استخدم to_int)
  boolean -> integer ممنوع

================================================================================
                    الباب الخامس: المتغيرات
================================================================================

5.1 الإعلان العام
-----------------
  variable : x=integer , y=real , name=string , flag=boolean .

5.2 المتغيرات المحلية (داخل الدوال)
------------------------------------
  function : compute ( a=integer ) return integer .
    variable : result=integer , temp=real .
    ...
  end_func .

5.3 المصفوفات
-------------
  // بحجم ثابت:
  variable : arr=integer[10] , names=string[5] .

  // بحرفية (تحدد الحجم):
  arr = [10, 20, 30] .
  names = ['Ali', 'Sara', 'Ahmed'] .

  // إعادة تعيين بحرفية (تغير الحجم):
  arr = [1, 2, 3, 4, 5] .

5.4 النطاقات
------------
  Global   : معلنة في variable قبل code
  Local    : معلنة داخل دالة
  الرؤية   : الدالة ترى المحلية أولًا ثم العامة
  التعديل  : SetVar تعدل في النطاق الأقرب

================================================================================
                    الباب السادس: التعبيرات
================================================================================

6.1 العوامل (من الأعلى للأدنى أسبقية)
--------------------------------------
  المستوى 1 : ( ) - تجميع
  المستوى 2 : - (أحادي) - سالب
  المستوى 3 : * / div mod - ضرب/قسمة
  المستوى 4 : + - جمع/طرح
  المستوى 5 : < > <= >= مقارنة
  المستوى 6 : == <> مساواة
  المستوى 7 : not نفي منطقي
  المستوى 8 : and و
  المستوى 9 : or أو

6.2 الحساب
----------
  2 + 3          = 5       (integer)
  2 + 3.5        = 5.5     (real)
  10 - 4         = 6
  3 * 4          = 12
  10 / 4         = 2.5     (real دائمًا)
  10 div 3       = 3       (قسمة صحيحة)
  10 mod 3       = 1       (باقي القسمة)
  -5 + 3         = -2

6.3 المقارنة
------------
  5 == 5         = true
  5 <> 3         = true
  3 < 5          = true
  5 > 3          = true
  5 <= 5         = true
  5 >= 3         = true

مقارنة النصوص بنفس الطريقة:
  'abc' == 'abc' = true
  'abc' < 'def'  = true

6.4 المنطق
----------
  true and false = false
  true or false  = true
  not true       = false

ملاحظة: لا يوجد short-circuit (الطرفان يُقيّمان دائمًا).

6.5 دمج النصوص
--------------
  'Hello' + ' ' + 'World'   = 'Hello World'
  'count: ' + to_string(5)  = 'count: 5'

ممنوع: 'count: ' + 5 (استخدم to_string)

6.6 استدعاء الدوال
------------------
  result = add(3, 4) .
  write('Hello') .

6.7 الفهرسة (0-based)
---------------------
  arr[0]               العنصر الأول
  arr[i + 1]           فهرس محسوب
  arr[length(arr) - 1] العنصر الأخير

================================================================================
                    الباب السابع: التحكم في التدفق
================================================================================

7.1 الشرط if
------------
  if cond then
    ...
  else
    ...
  end_if .

بدون else:
  if x > 0 then
    write('positive') .
  end_if .

متداخل:
  if age < 18 then
    write('minor') .
  else
    if age < 65 then
      write('adult') .
    else
      write('senior') .
    end_if .
  end_if .

7.2 الحلقة while
----------------
  while condition do
    ...
  end_while .

مثال:
  i = 1 .
  while i <= 5 do
    write(i) .
    i = i + 1 .
  end_while .

7.3 الحلقة for
--------------
  for i = start to end do
    ...
  end_for .

مع خطوة:
  for i = 0 to 10 step 2 do
    ...
  end_for .

  for i = 10 to 1 step -1 do
    ...
  end_for .

ملاحظة: start/end/step يجب أن تكون integer. الحلقة شاملة (تشمل end).

7.4 break و continue
--------------------
  for i = 1 to 10 do
    if i == 5 then
      break .
    end_if .
    if i mod 2 == 0 then
      continue .
    end_if .
    write(i) .
  end_for .

7.5 return
----------
  function : square ( x=integer ) return integer .
    return x * x .
  end_func .

للدوال void:
  function : greet ( name=string ) return void .
    write('Hello, ', name) .
    return .
  end_func .

return مبكر:
  function : check ( n=integer ) return string .
    if n < 0 then return 'negative' . end_if .
    if n == 0 then return 'zero' . end_if .
    return 'positive' .
  end_func .

================================================================================
                    الباب الثامن: الدوال
================================================================================

8.1 التعريف
-----------
  function : name ( p1=type1 , p2=type2 ) return returnType .
    ...body...
  end_func .

8.2 المعاملات
-------------
  - بالقيمة (pass-by-value) لكل الأنواع الأساسية
  - المصفوفات تمرر بالمرجع (pass-by-reference)

8.3 الاستدعاء الذاتي
--------------------
  function : factorial ( n=integer ) return integer .
    if n <= 1 then return 1 . end_if .
    return n * factorial(n - 1) .
  end_func .

8.4 الاستدعاء المتبادل
----------------------
  function : is_even ( n=integer ) return boolean .
    if n == 0 then return true . end_if .
    return is_odd(n - 1) .
  end_func .

  function : is_odd ( n=integer ) return boolean .
    if n == 0 then return false . end_if .
    return is_even(n - 1) .
  end_func .

ملاحظة: لا حاجة لتعريف مسبق. الترتيب لا يهم.

8.5 دوال void
-------------
  function : log_msg ( msg=string ) return void .
    write('[LOG] ', msg) .
    return .
  end_func .

  code :
    log_msg('Starting') .
  end .

8.6 الحدود
----------
  عدد المعاملات  : غير محدود عمليًا
  عمق الاستدعاء  : محدود بحجم Stack (~1MB)
  نوع الإرجاع    : أي نوع (ما عدا المصفوفات حتى الآن)

================================================================================
                    الباب التاسع: المصفوفات
================================================================================

9.1 الإعلان
-----------
  variable : a=integer[10] , b=real[5] , c=string[3] .

9.2 الحرفيات
------------
  a = [1, 2, 3, 4, 5] .
  b = [1.5, 2.5, 3.5] .
  c = ['one', 'two', 'three'] .
  d = [true, false, true] .

9.3 الفهرسة (0-based)
---------------------
  x = a[0] .              العنصر الأول
  a[2] = 10 .             تعديل العنصر الثالث
  i = length(a) - 1 .     الفهرس الأخير

9.4 الحجم
---------
  n = length(a) .

9.5 المرور للدوال
-----------------
  function : sum ( arr=array_integer ) return integer .
    variable : i=integer , total=integer .
    total = 0 .
    for i = 0 to length(arr) - 1 do
      total = total + arr[i] .
    end_for .
    return total .
  end_func .

  code :
    a = [1, 2, 3, 4, 5] .
    write(sum(a)) .   // 15
  end .

المصفوفات تمرر بالمرجع (التعديل يؤثر على الأصل).

9.6 حماية من الأخطاء
--------------------
  - فهرس خارج الحدود -> خطأ وقت تشغيل
  - عنصر بنوع خاطئ -> خطأ نوع
  - تعيين مصفوفة لمتغير مفرد -> خطأ

================================================================================
                    الباب العاشر: الإدخال والإخراج
================================================================================

10.1 الطباعة write
------------------
  write('Hello') .              Hello + سطر جديد
  write('Name: ', name) .       Name: Ahmed + سطر جديد
  write(42) .                   42 + سطر جديد
  write(a) .                    [1, 2, 3] + سطر جديد

كل write يضيف سطرًا جديدًا تلقائيًا.

10.2 القراءة read
-----------------
  variable : name=string , age=integer .

  code :
    write('Enter your name:') .
    read(name) .
    write('Enter your age:') .
    read(age) .
    write('Hello ', name, ', age ', age) .
  end .

التحويل التلقائي حسب نوع المتغير:
  string  -> السطر كاملًا
  integer -> يرفض abc
  real    -> يقبل 3.14
  boolean -> true/false/1/0

10.3 الملفات (مكتبة io)
-----------------------
  library : io .

  read_file(path)          قراءة الملف كاملًا (UTF-8)
  write_file(path, content) كتابة (يستبدل)
  append_file(path, content) إضافة
  file_exists(path)        هل الملف موجود؟
  delete_file(path)        حذف (يرجع true/false)

================================================================================
                    الباب الحادي عشر: المكتبات
================================================================================

11.1 الفرق بين native و function و extern
------------------------------------------
  native   : كود دلفي في المترجم (CallBuiltin)
  function : كود BMA خالص
  extern   : استدعاء DLL عبر FFI

11.2 تحميل المكتبات
-------------------
  library : math , strings , io , collections .

ترتيب البحث:
  1. BMA_LIBRARY_PATH (متغير بيئة)
  2. .\library\ (بجوار BMA_Compiler.exe)
  3. .\..\..\library\
  4. .\library\ (المجلد الحالي)

11.3 إنشاء مكتبة جديدة
----------------------
  أنشئ library/mylib.bmalib:

  BMA_Lib : mylib .
  native : some_func ( x=integer ) return integer .
  function : my_helper ( x=integer ) return integer .
    return x * 2 .
  end_func .
  end_lib .

  استخدمها:
  BMA_Pro : demo .
  library : mylib .
  code :
    write(my_helper(5)) .
  end .

================================================================================
                    الباب الثاني عشر: FFI
================================================================================

12.1 الإعلان
------------
  extern_lib : 'user32.dll' .

  extern : function_name ( p1=type1 , p2=type2 ) return returnType from 'ActualName' stdcall .

  from      : اسم الدالة الحقيقي في DLL
  stdcall   : اتفاقية الاستدعاء (افتراضي)
  cdecl     : بديل لـ مكتبات C

12.2 الأنواع المدعومة
---------------------
  BMA       | C/Windows    | ملاحظة
  ----------|--------------|------------------
  integer   | int32, DWORD | 32-bit
  boolean   | BOOL         | 0/1
  string    | char*        | ANSI
  wstring   | wchar_t*     | Unicode
  pointer   | void*        | مؤشر خام
  void      | void         | لا قيمة

12.3 الحدود
-----------
  - الحد الأقصى 8 معاملات لكل extern
  - لا structs حتى الآن
  - لا callbacks حتى الآن

12.4 أمثلة
----------

MessageBox:
  extern_lib : 'user32.dll' .
  extern : msg ( hwnd=integer , text=string , caption=string , flags=integer ) return integer from 'MessageBoxA' stdcall .

SQLite:
  extern_lib : 'sqlite3.dll' .
  extern : open ( path=string , db_out=pointer ) return integer from 'sqlite3_open' cdecl .
  extern : close ( db=pointer ) return integer from 'sqlite3_close' cdecl .
  extern : exec ( db=pointer , sql=string , callback=pointer , arg=pointer , errmsg=pointer ) return integer from 'sqlite3_exec' cdecl .

================================================================================
                    الباب الثالث عشر: إدارة الذاكرة
================================================================================

13.1 الدوال الأساسية (متاحة دائمًا)
------------------------------------
  alloc(bytes)       تخصيص ذاكرة مصفرة
  free(ptr)          تحرير الذاكرة
  peek_int(ptr)      قراءة int32
  poke_int(ptr, v)   كتابة int32
  peek_ptr(ptr)      قراءة مؤشر
  poke_ptr(ptr, v)   كتابة مؤشر

13.2 لماذا نحتاجها؟
-------------------
لأن دوال C أحيانًا تأخذ T** (مؤشر لمؤشر) أو T* (بنية).

مثال sqlite3_open:
  // في C:
  sqlite3 *db;
  sqlite3_open("file.db", &db);

  // في BMA:
  slot = alloc(4) .                مساحة لمؤشر
  open('file.db', slot) .          sqlite3 يكتب في slot
  db = peek_ptr(slot) .            نقرأ المؤشر
  free(slot) .                     نحرر

13.3 القواعد الأمنية
--------------------
  1. كل alloc يجب أن يقابله free
  2. لا تمرر مؤشر alloc لدالة تحتفظ به
  3. لا تستخدم free على مؤشر أعادته DLL
  4. تحقق من العائد قبل الاستخدام
  5. لا تستخدم مؤشرًا بعد free

================================================================================
                    الباب الرابع عشر: المكتبة القياسية
================================================================================

14.1 math
---------
  library : math .

  pi()             3.14159...
  e()              2.71828...
  sqrt(x)          جذر تربيعي (x >= 0)
  abs(x)           قيمة مطلقة
  sin(x), cos(x), tan(x)   مثلثات (راديان)
  pow(x, y)        x مرفوع لـ y
  floor(x)         تقريب لأسفل (integer)
  ceil(x)          تقريب لأعلى
  round(x)         أقرب عدد صحيح
  min(a, b)        الأصغر
  max(a, b)        الأكبر
  log(x)           ln(x) (x > 0)
  log10(x)         log10(x)
  exp(x)           e^x
  atan2(y, x)      arctan(y/x)
  random()         [0, 1)
  random_int(n)    [0, n)

14.2 strings
------------
  library : strings .

  length(s)                     طول
  upper(s), lower(s)            تحويل حالة
  trim(s)                       حذف مسافات
  substring(s, start, len)      مقطع
  index_of(s, sub)              الموقع (-1 = غير موجود)
  contains(s, sub)              يحتوي؟
  starts_with(s, p)             يبدأ بـ؟
  ends_with(s, s2)              ينتهي بـ؟
  replace(s, old, new)          استبدال
  char_at(s, i)                 الحرف رقم i
  repeat_str(s, n)              تكرار
  reverse(s)                    عكس
  split(s, sep)                 -> array_string
  join(arr, sep)                -> string
  newline()                     CRLF
  tab()                         \t
  to_string(n)                  عدد -> نص
  to_int(s)                     نص -> عدد
  is_empty(s)                   s == ''
  equals_ignore_case(a, b)      مقارنة بدون حالة
  count_char(s, c)              عدد مرات c في s
  is_palindrome(s)              هل متماثل؟

14.3 io
-------
  library : io .

  read_file(path)          -> string
  write_file(path, content)
  append_file(path, content)
  file_exists(path)        -> boolean
  delete_file(path)        -> boolean

14.4 collections
----------------
  library : collections .

  find(arr, v)             -> integer (فهرس أو -1)
  contains_array(arr, v)   -> boolean
  min_array(arr)           -> أصغر عنصر
  max_array(arr)           -> أكبر عنصر
  sum_array(arr)           -> مجموع
  reverse_array(arr)       -> void (عكس في المكان)
  sort(arr)                -> void (ترتيب تصاعدي)
  sort_desc(arr)           -> void (ترتيب تنازلي)

14.5 windows
------------
  library : windows .

  message_box(text, caption)   -> integer
  sleep(ms)                    -> void
  beep(freq, duration)         -> void
  get_tick_count()             -> integer

14.6 user32 (FFI)
-----------------
  library : user32 .

  message_box_a(hwnd, text, caption, flags)   -> integer (ANSI)
  message_box_w(hwnd, text, caption, flags)   -> integer (Unicode)

14.7 sqlite (FFI)
-----------------
  library : sqlite .

  open(path, slot)             -> integer
  close(db)                    -> integer
  exec(db, sql, cb, arg, err)  -> integer

================================================================================
                    الباب الخامس عشر: الأمثلة الكاملة
================================================================================

15.1 Hello World
----------------
BMA_Pro : hello .
code :
  write('Hello, BMA!') .
end .

15.2 Calculator
---------------
BMA_Pro : calc .
library : strings .
variable : a=real , b=real , op=string , result=real .

function : compute ( x=real , y=real , operation=string ) return real .
  if operation == '+' then return x + y . end_if .
  if operation == '-' then return x - y . end_if .
  if operation == '*' then return x * y . end_if .
  if operation == '/' then
    if y == 0 then
      write('Error: div by zero') .
      return 0 .
    end_if .
    return x / y .
  end_if .
  write('Unknown op: ', operation) .
  return 0 .
end_func .

code :
  write('Enter first number:') .
  read(a) .
  write('Enter op (+ - * /):') .
  read(op) .
  write('Enter second number:') .
  read(b) .
  result = compute(a, b, op) .
  write(a, ' ', op, ' ', b, ' = ', result) .
end .

15.3 Array Sorting
------------------
BMA_Pro : sort_demo .
library : collections .

variable : a=integer[10] .

code :
  a = [42, 17, 93, 8, 55, 23, 71, 4, 88, 31] .
  write('Before: ', a) .
  sort(a) .
  write('Sorted: ', a) .
  write('Sum: ', sum_array(a)) .
  write('Min: ', min_array(a)) .
  write('Max: ', max_array(a)) .
end .

15.4 File Processing
--------------------
BMA_Pro : file_demo .
library : strings , io .

variable : content=string , lines=string[100] , i=integer .

code :
  write_file('data.txt', 'line1' + newline() + 'line2' + newline() + 'line3') .
  content = read_file('data.txt') .
  lines = split(content, newline()) .
  for i = 0 to length(lines) - 1 do
    write('Line ', i + 1, ': ', lines[i]) .
  end_for .
end .

15.5 Windows MessageBox (FFI)
-----------------------------
BMA_Pro : msg_demo .
library : user32 .

code :
  message_box_a(0, 'Hello from BMA!', 'Test', 0) .
  message_box_w(0, 'Hello', 'Unicode Test', 0) .
end .

15.6 SQLite (FFI)
-----------------
BMA_Pro : sqlite_demo .
library : strings , sqlite .

variable : slot=pointer , db=pointer , rc=integer .

code :
  slot = alloc(4) .
  rc = open('test.db', slot) .
  db = peek_ptr(slot) .
  rc = exec(db, 'CREATE TABLE IF NOT EXISTS users (id INTEGER, name TEXT)', 0, 0, 0) .
  rc = exec(db, 'INSERT INTO users VALUES (1, ''Ahmed'')', 0, 0, 0) .
  rc = close(db) .
  free(slot) .
  write('Done!') .
end .

================================================================================
                    الباب السادس عشر: القواعد الذهبية
================================================================================

16.1 النحو
----------
  1. كل جملة تنتهي بنقطة .
  2. السلاسل النصية بـ '...' فقط (وليس "...")
  3. الكلمات المحجوزة lowercase
  4. لا ;، لا {}، لا () حول الشروط
  5. داخل السلسلة: '' تعني ' حرفية
  6. التعليقات: // و /* */

16.2 الأنواع
------------
  1. لا تحويل تلقائي integer -> string (استخدم to_string)
  2. لا تحويل تلقائي real -> integer (استخدم floor/ceil/round)
  3. integer -> real تلقائي (آمن)
  4. == للمقارنة، = للتعيين

16.3 الأسماء
------------
  1. حساسة لحالة الأحرف (x != X)
  2. تبدأ بحرف أو _ وتحتوي حروف/أرقام/_
  3. لا يمكن أن تكون كلمة محجوزة

16.4 المصفوفات
--------------
  1. 0-based (الفهرس الأول = 0)
  2. length(arr) يرجع الحجم الحالي
  3. حرفية [...] تغير الحجم
  4. مرور بالمرجع للدوال

================================================================================
                    الباب السابع عشر: القيود المعروفة
================================================================================

17.1 غير مدعوم بعد
------------------
  الميزة                | البديل
  ----------------------|---------------------------
  class / OOP           | مصفوفات + دوال
  try / catch           | تحقق يدوي
  struct                | مصفوفة أو متغيرات
  enum                  | ثوابت integer
  Generics              | array_*
  Lambdas / closures    | غير متاح
  switch / case         | if/else
  null / nil            | قيم افتراضية
  String interpolation  | +
  Multiple returns      | مصفوفة
  Default parameters    | دوال متعددة
  Varargs               | مصفوفة
  for each              | for i = 0 to length-1
  Range literals        | for
  Ternary ?:            | if/else
  ++ / --               | i = i + 1
  && / ||               | and / or
  !=                    | <>
  Escapes (\n, \t)      | newline(), tab()
  Multi-line strings    | newline() في +
  const declarations    | variable + تعليق
  # comments            | //

17.2 حدود FFI
-------------
  - 8 معاملات كحد أقصى
  - لا structs
  - لا callbacks
  - لا double / int64 / float

================================================================================
                    الباب الثامن عشر: خارطة التطوير
================================================================================

المرحلة | الميزة                              | الجهد
--------|-------------------------------------|------------
1       | OOP أساسي (class + new)             | 3-4 أسابيع
2       | struct                              | أسبوعان
3       | FFI structs (win32)                 | أسبوع
4       | Python bridge                       | أسبوع
5       | try/catch                           | ساعة
6       | enum, const                         | أسبوعان
7       | case / switch                       | أسبوع
8       | String escapes (\n)                 | 3 أيام
9       | Generics                            | 4 أسابيع
10      | Bytecode VM                         | 3 أشهر
11      | Self-hosting                        | سنة+

================================================================================
                    الباب التاسع عشر: نموذج الذكاء الاصطناعي
================================================================================

19.1 System Prompt كامل (الصقه في ChatGPT/Claude)
--------------------------------------------------

You are an expert BMA programmer. BMA is a programming language
created by Boutelis Mohamed El Amine. It combines the simplicity
of Python with the type system of Delphi, and supports FFI for
calling any DLL.

## Structure
BMA_Pro : program_name .
library : lib1 , lib2 .
variable : x=integer , s=string .
function : name ( p=type ) return type .
  ...
end_func .
code :
  ...
end .

## Absolute syntax rules
- Every statement ends with .
- Strings use '...' (single quotes only)
- Keywords are lowercase
- No ;, no {}, no () around conditions
- No !=, use <>
- No && / ||, use and / or
- No ++ / --, use i = i + 1
- Comments: // single-line, /* */ block
- Inside string, '' = literal '

## Types
integer, real, string, boolean, void
array_integer, array_real, array_string, array_boolean
pointer, wstring, int64, real32 (FFI only)

## Type rules
- No implicit int -> string: use to_string(n)
- No implicit real -> int: use floor/ceil/round
- int -> real automatic

## Control flow
if cond then ... else ... end_if .
while cond do ... end_while .
for i = a to b (step s) do ... end_for .
break .   continue .   return expr .

## Arrays (0-based)
variable : a=integer[10] .
a = [1, 2, 3] .
x = a[0] .
a[1] = 5 .
n = length(a) .

## I/O
write(expr, expr, ...)   // adds newline
read(var)

## Available libraries
math: pi, e, sqrt, abs, sin, cos, tan, pow, floor, ceil, round,
      min, max, log, log10, exp, atan2, random, random_int
strings: length, upper, lower, trim, substring, index_of, contains,
         starts_with, ends_with, replace, char_at, repeat_str, reverse,
         split, join, newline, tab, to_string, to_int, is_empty,
         equals_ignore_case, count_char, is_palindrome
io: read_file, write_file, append_file, file_exists, delete_file
collections: find, contains_array, min_array, max_array, sum_array,
             reverse_array, sort, sort_desc
windows: message_box, sleep, beep, get_tick_count
user32: message_box_a, message_box_w

## Not supported (do NOT use)
class, struct, enum, try/catch, generics, lambdas, null,
switch/case, string escapes (\n -> newline()), ternary, ++/--,
!= (use <>), &&, ||, #comments (use //), const

## Verification before output
- [ ] Every statement ends with .
- [ ] All variables declared
- [ ] All if/while/for have matching end_xxx
- [ ] No ;, {, }, !=, ++
- [ ] Strings in single quotes
- [ ] Type conversions explicit

Output ONLY code unless asked to explain. Wrap in ```bma fences.

19.2 أمثلة تعليمية للـ AI
--------------------------

// Minimal
BMA_Pro : hello .
code :
  write('Hello') .
end .

// Function
BMA_Pro : demo .
function : add ( a=integer , b=integer ) return integer .
  return a + b .
end_func .
code :
  write(add(3, 4)) .
end .

// Loop
BMA_Pro : loop .
variable : i=integer .
code :
  for i = 1 to 5 do
    write(i) .
  end_for .
end .

// Array
BMA_Pro : arr .
variable : a=integer[5] , i=integer .
code :
  a = [1, 2, 3, 4, 5] .
  for i = 0 to length(a) - 1 do
    write(a[i]) .
  end_for .
end .

================================================================================
                    الباب العشرون: الخاتمة والترخيص
================================================================================

20.1 معلومات الإصدار
--------------------
  اللغة         : BMA (Boutelis Mohamed El Amine)
  الإصدار       : 1.0.0
  التاريخ       : 2026
  المترجم       : BMA_Compiler.exe
  الامتدادات    : .bma , .bmalib
  الترميز       : UTF-8

20.2 الملفات الأساسية
---------------------
  BMA_Lexer.pas         ~350 سطر  تحويل النص إلى Tokens
  BMA_AST.pas           ~500 سطر  تعريف عقد الشجرة
  BMA_Parser.pas        ~700 سطر  بناء AST
  BMA_Interpreter.pas   ~1600 سطر تنفيذ AST
  BMA_FFI.pas           ~200 سطر  تحميل DLLs واستدعاء الدوال
  BMA_Compiler.dpr      ~100 سطر  نقطة الدخول CLI

20.3 المجلدات
-------------
  library/              مكتبات .bmalib
  examples/             أمثلة
  tests/                اختبارات

20.4 الترخيص
------------
  [يُحدد لاحقًا من قبل المؤلف]

20.5 المؤلف
-----------
  Boutelis Mohamed El Amine
  مؤسس لغة BMA

20.6 المساهمون
--------------
  مفتوح للمساهمات

================================================================================
                              انتهى الملف
                    BMA - لغة برمجة من الصفر، بلا حدود
================================================================================
