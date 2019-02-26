// rjesavanje sudokua metodom simuliranog kaljenja (simulated annealing)


#include <iostream>
#include <cmath>
#include <random>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>

using namespace std;

    //random generator
    auto seed = chrono::high_resolution_clock::now().time_since_epoch().count();
    mt19937 generator( seed );
    uniform_real_distribution<double> urd(0,1);     //uniformna distribucija iu [0,1>

int init(int stanje[9][9],int ulaz[9][9])           //popuni prazna polja td se svaki broj javlja samo jednom u svakom redu
{
    for(int i=0; i<9; ++i)
    {
        int pojavljivanja[9];       //pojavljivanja u redu
        for(int j =0; j<9; ++j) pojavljivanja[j]=0;
        for(int j =0; j<9; ++j)
        {
            if(stanje[i][j] != -1) pojavljivanja[stanje[i][j]-1]=1;  //broj na poziciji i,j se pojavio pa je na njegovom indeksu 1
        }

        for(int j=0,k=0 ; j<9 ;++j)  //k se mice u else
        {
            if(ulaz[i][j]==0)       //na ulazu je bila prazna kucica, popuni sa nekim brojem koji se ne javlja u tom redu
            {
                while(pojavljivanja[k] != 0) ++k; //nadi br koji se ne javlja u tom redu
                stanje[i][j]=k+1;       //brojevi su od 1..9 ineksi 0..8
                pojavljivanja[k]=1;
            }

        }
    }
}

int fcijena(int stanje[9][9], int cijena[9][9])        //cijena od elementa i,j je broj kršenja pravila sudokua
{
    int ukupnaCijena=0;

    for(int i=0; i<9; ++i)
    {
        for( int j=0; j<9; ++j)          //radim sa brojem stanje[i][j]
        {
            cijena[i][j]=0;             //prije racunanja cijene elementa postavi na 0

            for(int k=0; k<9; ++k)
            {
                if((stanje[i][k]==stanje[i][j] && k!=j) || (stanje[k][j]==stanje[i][j] && k!=i)) cijena[i][j]++;  //simultana provjera ponavljanja elemenata u retku i stupcu
            }
            //ponavljanja u 3x3 podmatrici
            int m=i/3, n=j/3;
            for(int k=0; k<3; ++k) for(int l=0; l<3; ++l) if(stanje[i][j] == stanje[3*m+k][3*n+l] &&  i!=3*m+k && j!= 3*n+1) cijena[i][j]++;

            ukupnaCijena+= cijena[i][j];        //ukupnu cijenu uvecaj za cijenu svakog elementa nakon obrade tog elementa
        }
    }
    return ukupnaCijena;

}

vector<int> sample(int cijena[9][9],int ulaz[9][9])
{


    double F[81];
    int idx=1;
    F[0]=exp(cijena[0][0]);
    for(int i=0; i<9; ++i)
        for(int j=0; j<9; ++j)
    {
        if(i==0 && j==0) continue;
        F[idx]=F[idx-1]+exp(cijena[i][j]);
        idx++;
    }


    for(int i=0; i<81; ++i) F[i]/= F[80]; //ukupne vrijednosti su sumiranje u F[80]


    vector<int> ret;
    int nadeni=0;
    while(nadeni<2)
    {
       double rnd=urd(generator);   //random broj
       int sample=0;
       while (rnd > F[sample]) sample++;     //nadi indeks za kojeg je distribucija veca od random broja
       int i = sample/9;
       int j = sample-9*i;
       if(ulaz[i][j]==0)  //odabrana pozicija nije fiksni broj sa ulaza
       {
           ret.push_back(i);        //nakon nalaska dva sampla ret izgleda (x1,y1,x2,y2) gdje su x1y1 koordinate prvog polja a x2y2 koordinate drugog polja
           ret.push_back(j);
           ++nadeni;
       }
    }

    return ret;
}


int main()
{

	int ulaz[9][9]; //matrica sa 0 i 1 gdje 1 stoji na poziciji na kojoj je ulazni broj koji nesmije mijenjati poziciju
	int stanje[9][9]; // aktualna matrica sudokua
	int cijena[9][9]; // matrica cijena za stanje
	int tempStanje[9][9]; // privremeno stanje, koristi se u MCMC
	int tempCijena[9][9]; // privremena cijena, koristi se u MCMC
	int faliBrojeva[9]; // brojaci koliko je kojih brojeva na ulazu, broj i se sprema na index i-1

	//postavljanje pocetnih nizova u pocetno stanje
	for(int i=0;i<9;++i) faliBrojeva[i]=9;		//fali 9 instanci svakog broja na pocetku
	for(int i=0;i<9;++i) for(int j=0;j<9;++j) { ulaz[i][j]=0; stanje[i][j]=-1 ;} //-1 oznacava da je polje prazno

	//ucitavanje sudoku tablice iz ulaza i postavljanje odg vrijednosti
	ifstream in("sudokuUlaz.txt");
	if(in.is_open())
	{
		for(int i=0; i<9 ; ++i){
			string linija;
			getline(in,linija);
			for(int j=0; j<9; ++j){
				if(linija[j] != '.'){		//ako polje na ulazu nije prazno postavi odg pozicije u pocetnim matricama
				int broj= linija[j] - '0';		//konverzija char u int, oduzimanje od '0' da ne bude ascii broj
				stanje[i][j]= broj;
				ulaz[i][j]= 1;
				faliBrojeva[broj-1]--;      //netreba nista ako je . jer je vec ostavljeno u pocetno prije
				}
			}
		}
	}else{ cout<<"nije se uspio otvorit sudokuUlaz.txt"<<endl; exit(-1); }

	init(stanje,ulaz);
    fcijena(stanje,cijena);

    //random number generator       aktivacija samo jednom, sakriva globalni generator tako da se globalni generator koristi samo u funkciji sample a ovaj odvojeni generator u mainu
    mt19937 generator(random_device{}());
    uniform_real_distribution<double> urd(0,1);

    cout<<"nakon popunjavanja ulaznog stanja sa random brojevima ploca izgleda :"<<endl;
    for(int i=0; i<9; ++i) {
            for(int j=0;j<9;++j) cout<<stanje[i][j]<<" "; cout<<endl;}
    cout<<endl;

    //iteriranje
    int ukupnaCijena;
    int iter=0,iter2=0;
    int brZamjena=0;
    bool rijesen=false;
    for(double T=5; iter<1000000; ++iter)        //1 000 000 iteracija je proizvoljno odabran gornji limit prije odustajanja      T je temperatura
    {
        for(int i=0; i<9; ++i) for(int j=0; j<9; ++j) tempStanje[i][j]=stanje[i][j];
        vector<int> poz=sample(cijena,ulaz);
        tempStanje[poz[0]][poz[1]]=stanje[poz[2]][poz[3]];      //zamjena elemenata u tempStanju
        tempStanje[poz[2]][poz[3]]=stanje[poz[0]][poz[1]];

        //računanje faktora alpha
        ukupnaCijena=fcijena(stanje,cijena);
        int tempUkupnaCijena= fcijena(tempStanje,tempCijena);
//        double a= exp((ukupnaCijena-tempUkupnaCijena)/T);    //verzija sa temperaturom
        double a= exp(ukupnaCijena-tempUkupnaCijena);


        //odlucivanje o prihvacanju

        if(a >=1)   //prihvati zamjenu
        {
            stanje[poz[0]][poz[1]]=tempStanje[poz[0]][poz[1]];  //stanje i tempStanje se razlikuju u samo dvije pozicije
            stanje[poz[2]][poz[3]]=tempStanje[poz[2]][poz[3]];
            brZamjena++;
            ukupnaCijena=tempUkupnaCijena;
        }else if(a >= urd(generator))   //prihvati zamjenu sa vjerojatnosti a
        {
            stanje[poz[0]][poz[1]]=tempStanje[poz[0]][poz[1]];
            stanje[poz[2]][poz[3]]=tempStanje[poz[2]][poz[3]];
            brZamjena++;
            ukupnaCijena=tempUkupnaCijena;
        }

        //ako je nista od navedenog ignorira se odabrana zamjena i nastavlja se sa petljom

//       if(iter%1000==0) T*=0.99;  //smanji temperaturu svakih 50 iteracija za 1%
        if(iter%1000==0)cout<<"iteracija: "<<iter<<" cijena je: "<<ukupnaCijena<<" a je :"<<a<<endl;
        if(ukupnaCijena==0){
                rijesen=true;
                break;      //nasli smo rjesenje sudokua
        }
 //       if(T<1){iter2++;}
 //       if(iter2==100000) {
 //               T=3;         //ako si 10 000 iteracija u temp ispod 1 i nisi nasao rjesenje onda ni neces, vrati temp na pocetak
 //               iter2=0;
 //       }
    }

    cout<<endl<<"sudoku tablica na kraju izgleda:"<<endl<<endl;

    for(int i=0; i<9; ++i) {
            for(int j=0;j<9;++j) cout<<stanje[i][j]<<" "; cout<<endl;}
    cout<<endl<<endl<<"ukupna cijena je: "<<ukupnaCijena<<" broj iteracija koje je prosao program je: "<<iter<<" a broj zamjena je: "<<brZamjena<<endl;

    system("PAUSE");
	return 0;
	};
