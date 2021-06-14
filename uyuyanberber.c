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

if(argc!=2)   //eğer program başlatılırken argüman girilmemişse ya da birden fazla argüman girilmişse ekrana parçalama hatası yazdırılır, örn ./uyuyanberber , veya ./uyuyanberber 5 4, veya ./uyuyanberber 5 1 9 gibi hatalar
{
printf("Parçalama hatası!\n\n");
return EXIT_FAILURE;
}

musteriSayi=atoi(args[1]);   //kullanıcıdan alınan müşteri sayısını atoi ile musteriSayi değişkenine atama
bosBeklemeKoltugu=beklemeKoltugu;
koltuk=(int*) malloc(sizeof(int)*beklemeKoltugu);   //bellek yönetimi

if(musteriSayi>SALON_KAPASITE)   //salon kapasitesinden fazla bir müşteri sayısı girilirse ekrana sınır değeri yazdırılır, çıkış yapar
{
printf("\nSalon kapasitesi: %d\n",SALON_KAPASITE);
return EXIT_FAILURE;
}

//kullanıcıdan alınan müşteri sayısının ve define tanımlanan verilerin ekrana yazdırılması
printf("\nMüşteri sayısı: %d\n\n", musteriSayi);
printf("Bekleme koltuğu sayısı: %d\n\n", beklemeKoltugu);   //sabit bekleme koltuk sayısı
printf("Traş koltuğu sayısı: %d\n", trasKoltugu);   //sabit traş koltuğu sayısı
printf("\n");

//threadlerin oluşturulması
pthread_t berberler[trasKoltugu];
pthread_t musteriler[musteriSayi];   

//semaforların ilklendirilmesi
sem_init(&sem_berber, 0, 0);   //berber semaforuna ilk değer 
sem_init(&sem_musteri, 0, 0);   //müşteri semaforuna ilk değer
sem_init(&sem_mutex, 0, 1);   //mutex semaforuna ilk değer

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
printf("\n Tüm müşterilerin traşı tamamlandı, dükkan kapandı. Berberler eve döndü..\n");
return EXIT_SUCCESS;
}

void MusteriOlusturma()
{
//ms cinsinden random aralıklarla müşteri oluşturma
srand((unsigned int)time(NULL));
usleep(rand()%(100000-25000+1)+25000);
}


void Berber (void* sayi)
{
int x=*(int*)sayi+1;   //1. ve 2. berber belirten değişken
int musteriId;
int nextMusteri;

printf("%d. Berber dükkana geldi.\n", x);

while(2==2)
{ //salonda müşteri yoksa berber uyumaya gider
if(!musteriId)  printf("%d. Berber uyumaya gitti.\n", x);


sem_wait(&sem_berber);   //uyuyan berber kuyruğuna katılma
sem_wait(&sem_mutex);   //koltuk kilitleme

//traş edilecek müşteriyi seçme
salonMusteri = (++salonMusteri)%beklemeKoltugu;
nextMusteri = salonMusteri;
musteriId=koltuk[nextMusteri];
//iş parçacığının id’sini döndürme
koltuk[nextMusteri]=pthread_self();   

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
int doluKoltuk;
int berberId;   //berberleri ayırt etmek için

sem_wait(&sem_mutex);   //başlangıçta oturulmaması için koltuk kilitleme
printf("%d. Müşteri dükkana geldi..\n", x);

if(bosBeklemeKoltugu>0)   //bekleme salonunda boş koltuk varsa
{
//gelen müşteriyi bekleme salonuna alıp boş koltuk sayısını azaltma
bosBeklemeKoltugu--;
printf("%d. Müşteri bekleme salonunda bekliyor.\n", x);

//müşterinin bekleme salonunda oturacağı koltuğu seçme
musaitKoltuk=(++musaitKoltuk)%beklemeKoltugu;
doluKoltuk=musaitKoltuk;
koltuk[doluKoltuk]=x;

sem_post(&sem_mutex);   //koltuk kilidini kaldırma
sem_post(&sem_berber);   //berberi uyandırma

sem_wait(&sem_musteri);   //bekleyen müşteri kuyruğuna katılma
sem_wait(&sem_mutex);   //koltuk kilitleme

//müşteriyi traş koltuğuna oturtma
berberId=koltuk[doluKoltuk];
bosBeklemeKoltugu++;

sem_post(&sem_mutex);   //işlemler tamamlandı, koltuk kilidini kaldır
}
else   //bekleme salonu doluysa, müşteriyi geri gönderme
{
sem_post(&sem_mutex);
printf("%d. Müşteri bekleme salonunda yer bulamadı, dükkandan çıktı..\n", x);
}pthread_exit(0);
}
