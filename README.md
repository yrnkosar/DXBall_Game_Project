# DXBall_Game_Project
 Microcontroller-based game project
Projemiz, DX-Ball tarzında bir oyun makinesi geliştirmeyi amaçlamaktadır ve bu proje Arduino IDE ile Proteus programları kullanılarak oluşturulmuştur.

Oyunda, oyuncu bir fiziksel palet kontrol cihazını kullanarak topu yönlendirir ve OLED ekrandaki tuğlaları kırmaya çalışır. Palet kontrolü, potansiyometre ile sağlanmaktadır. Oyun başladığında, OLED ekranda "Başlat" ve "Çıkış" seçeneklerinin bulunduğu bir menü görülür. Menüyü kontrol etmek için Proteus devresine bağlanmış üç buton kullanılır: biri yukarı, diğeri aşağı, üçüncüsü ise seçim yapma butonudur. Oyuncu "Başlat" seçeneğine bastığında "Oyun Başlıyor…" mesajı görüntülenir ve birinci seviyeye geçilir. "Çıkış" seçeneği seçildiğinde ise "Oyunumuza gösterdiğiniz ilgi için teşekkür ederiz." mesajı gösterilir ve oyun sonlanır.

Oyun başladığında, oyuncu potansiyometre ile topu yönlendirerek tuğlaları kırmaya başlar. Her kırılan tuğla 1 puan kazandırır. Ayrıca, her tuğla kırıldığında %10 ihtimalle bir obje düşer. Oyuncu bu objeyi aldığında ekstra can kazanır ve devredeki sönmüş LED tekrar yanar. Oyuncu, birinci seviyedeki tüm tuğlaları kırdığında, 3 saniyelik bir bekleme süresinin ardından ikinci seviyeye geçilir. Her seviyede farklı tuğla düzenleri bulunmaktadır. Oyuncu, tüm canlarını kaybedene kadar oyun devam eder. Oyun sona erdiğinde, ekranda "Oyunu kaybettiniz" ve "Skor" mesajı 5 saniye boyunca gösterilir ve ardından ana menüye geri dönülür.

Bu proje, DX-Ball gibi klasik tuğla kırma oyunlarına benzer bir oyun deneyimi sunmak için fiziksel donanım ve mikrodenetleyici kontrolünü birleştiren bir yapıya sahiptir.
