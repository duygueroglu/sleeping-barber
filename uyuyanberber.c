#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define trasKoltugu 2   //berber koltuğu sayısını define olarak tanımlama
#define beklemeKoltugu 5   //bekleme koltuğu sayısını define olarak tanımlama
#define SALON_KAPASITE 20   //salon kapasitesi belirleme
#define SHAVING_TIME 2   //traş süresi

//semafor tanımlama
sem_t sem_berber;
sem_t sem_musteri;
sem_t sem_mutex;

//fonksiyonları tanımlama
void Berber (void* sayi);
void Musteri (void* sayi);
void MusteriOlusturma();

//değişkenleri tanımlama
int musteriSayi=0;
int bosBeklemeKoltugu=0;
int salonMusteri=0;
int musaitKoltuk=0;
int* koltuk;

int main(int argc, char** args)
{
if(argc!=2)
{
printf("Error!\n\n");
return EXIT_FAILURE;
}

musteriSayi=atoi(args[1]);   //kullanıcıdan müşteri sayısını atoi ile değişkene atama
bosBeklemeKoltugu=beklemeKoltugu;
koltuk=(int*) malloc(sizeof(int)*beklemeKoltugu);   //bellek yönetimi

if(musteriSayi>SALON_KAPASITE)   //salon kapasitesinden fazla bir müşteri sayısı girilirse uygulamayı çalıştırmadan çıkış yapıyor
{
printf("\nMüşteri sınırı: %d\n\n",SALON_KAPASITE);
return EXIT_FAILURE;
}

//kullanıcıdan alınan müşteri sayısı ve define tanımlanan verilerin ekrana yazdırılması
printf("\nMüşteri sayısı: %d",musteriSayi);
printf("\nBekleme koltuğu sayısı: %d",beklemeKoltugu);
printf("\nTraş koltuğu sayısı: %d\n\n",trasKoltugu);

pthread_t berberler[trasKoltugu], musteriler[musteriSayi];   //thread oluşturma

//semaforları ilklendirme
sem_init(&sem_berber, 0, 0);
sem_init(&sem_musteri, 0, 0);
sem_init(&sem_mutex, 0, 1);

printf("Dükkan açıldı..\n\n");

int i=0;

//berber threadinin oluşturulması
for(i=0; i<trasKoltugu; i++)
{
pthread_create(&berberler[i], NULL, (void*)Berber, (void*)&i);
sleep(2);
}

//müşteri threadinin oluşturulması
for(i=0; i<musteriSayi; i++)
{
pthread_create(&musteriler[i], NULL, (void*)Musteri, (void*)&i);
MusteriOlusturma();
}


for(i=0; i<musteriSayi; i++)
{
pthread_join(musteriler[i], NULL);   //tüm müşterilerin işlemi bittikten sonra threadin kapatılması
}

printf("\n Tüm müşterilerin traşı tamamlandı, dükkan kapandı. Berberler eve döndü..\n\n");

return EXIT_SUCCESS;
}


void Berber (void* sayi)
{
int x=*(int*)sayi+1;   //1. ve 2. berber belirten değişken
int siradakiMusteri, musteriId;

printf("%d. Berber dükkana geldi.\n", x);

while(2==2)
{
if(!musteriId)   //salonda müşteri yoksa berber uyumaya gider
{
printf("%d. Berber uyumaya gitti.\n\n", x);
}

sem_wait(&sem_berber);   //uyuyan berber kuyruğuna katılma
sem_wait(&sem_mutex);   //koltuk kilitleme

//traş edilecek müşteriyi seçme
salonMusteri = (++salonMusteri) % beklemeKoltugu;
siradakiMusteri = salonMusteri;
musteriId=koltuk[siradakiMusteri];
koltuk[siradakiMusteri]=pthread_self();   //iş parçacığının id'sini döndürme

sem_post(&sem_mutex);   //koltuk kilidini kaldırma
sem_post(&sem_musteri);   //müşterinin traşına başlama

//işlemlerin ekrana yazdırılması
printf("%d. Berber, %d. müşteriyi traş etmeye başladı.\n", x, musteriId);
sleep(SHAVING_TIME);
printf("%d. Berber, %d. müşterinin traşını bitirdi.\n", x, musteriId);

}
}

void Musteri (void* sayi)
{
int x=*(int*)sayi+1;   //hangi müşterinin olduğunu belirten değişken
int oturulanKoltuk, berberId;

sem_wait(&sem_mutex);   //başlangıçta oturulmaması için koltuk kilitleme

printf("%d. Müşteri dükkana geldi..\n", x);

if(bosBeklemeKoltugu>0)   //bekleme salonunda boş koltuk varsa
{
//gelen müşteriyi bekleme salonuna alıp boş koltuk sayısını azaltma
bosBeklemeKoltugu--;
printf("%d. Müşteri bekleme salonunda bekliyor.\n", x);

//müşterinin bekleme salonunda oturacağı koltuğu seçme
musaitKoltuk=(++musaitKoltuk)%beklemeKoltugu;
oturulanKoltuk=musaitKoltuk;
koltuk[oturulanKoltuk]=x;

sem_post(&sem_mutex);   //koltuk kilidini kaldırma
sem_post(&sem_berber);   //berberi uyandırma

sem_wait(&sem_musteri);   //bekleyen müşteri kuyruğuna katılma
sem_wait(&sem_mutex);   //koltuk kilitleme

//müşteriyi traş koltuğuna oturtma
berberId=koltuk[oturulanKoltuk];
bosBeklemeKoltugu++;

sem_post(&sem_mutex);   //işlemler tamamlandı, koltuk kilidini kaldır
}
else   //bekleme salonu doluysa, müşteriyi geri gönderme
{
sem_post(&sem_mutex);
printf("%d. Müşteri bekleme salonunda yer bulamadı, dükkandan ayrılıyor..\n\n", x);
}
pthread_exit(0);
}

void MusteriOlusturma()
{
//ms cinsinden random aralıklarla müşteri oluşturma
srand((unsigned int)time(NULL));
usleep(rand()%(100000-25000+1)+25000);
}