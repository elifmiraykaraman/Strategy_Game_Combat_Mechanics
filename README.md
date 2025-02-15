Proje, İnsan İmparatorluğu ve Ork Lejyonu arasında geçen bir savaş simülasyonunu gerçekleştirmektedir.

Savaş, JSON formatındaki veriler kullanılarak birimlerin saldırı, savunma, kritik vuruş, yorgunluk ve araştırmalar gibi faktörlere göre hesaplanmaktadır. Tüm savaş süreci adım adım kaydedilerek çıktılar oluşturulmaktadır.

Projede C dili kullanılmış olup, savaş mekanikleri detaylı hesaplamalarla modellenmiştir. Çalıştırıldığında, savaşın başlama ve bitiş durumu terminalde görüntülenebilir, ayrıca detaylı sonuçlar bir dosyaya kaydedilir.

Projeyi çalıştırmak için:

gcc src/*.c -o battle_simulator
./battle_simulator
cat output/savas_sim.txt

Tüm veriler Files/ klasöründeki JSON dosyalarından okunmaktadır.

Bu proje, temel savaş simülasyonları ve algoritma geliştirme konularında çalışma yapmak isteyenler için uygundur.

