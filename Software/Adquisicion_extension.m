clear variables; clc; close all;

% s = serialport("COM10",9600,'Stopbits',2);
%     
% tension=zeros;
% i=1; 

disp('Cte. de tiempo iguales --> 1');
disp('Cte. de tiempo diferen --> 2');
cte=input("Ingrese opción: ");
if (cte==1)
    archivo='cte_iguales.mat';
elseif (cte==2)
    archivo='cte_distintas.mat';
else
    error('Error.');
end

    
disp('Presione enter para comenzar la adquisiciónde datos...');
pause();
% flush(s,"output");
% write(s,1,"CHAR"); % Inicia adquisicion de datos
% disp('Muestreando...');
% tic;
% tf=0;
% while tf<5
%     tension(1,i)=read(s,1,"UINT16"); % Leo el puerto serie.
%     i=i+1;
%     tf=toc;
% end
% flush(s,"output");
% write(s,1,"CHAR"); % Finaliza adquisicion de datos
% clear s;
disp('Adquisición de datos finalizada.');

% Procesamiento de señal
% tension=tension*5/1023;   % Convierto a tensión.
% t=
load(archivo);
t=datos.time;
tension=datos.V;

% Grafico de respuesta al escalon
figure('Name','Datos adquiridos');
plot(t,tension,'r','Linewidth',2); grid on; grid minor;
title('Rta al escalón.');
xlabel('Tiempo[seg]','Interpreter','Latex');
ylabel('V(t)[V]','Interpreter','Latex');
legend('Muestras')

% Procesamiento FOPDT
figure('Name','FOPDT');
FT=fittype("Kp*(1-exp(-(x-L)/T)).*(x>L)");
FOPDT=fit(t',tension',FT,'StartPoint',[0.01 0.01 0.01]);
plot(FOPDT,t',tension');grid on; grid minor;
title('FOPDT model');
xlabel('Tiempo[seg]','Interpreter','Latex');
ylabel('V(t)[V]','Interpreter','Latex');
legend('Muestra','Aproximación','Location','SouthEast');
parametros=coeffvalues(FOPDT);
Kp1=parametros(1);
L1=parametros(2);
T1=parametros(3);
fprintf('\n\nLista de parámetros para la aproximación FOPDT:\n');
fprintf('\tKp=%.2i \n',Kp1);
fprintf('\tL=%.2i \n',L1);
fprintf('\tT=%.2i \n',T1);
fprintf('y(t)=%.2i*(1-exp(-(t-%.2i)/%.2i))*u(t-%.2i)\n',Kp1,L1,T1,L1);

Sal_FOPDT=FOPDT(t)';
ec=(tension-Sal_FOPDT).^2;
fprintf("\nIndice de desempeño FOPDT:\n");
ISE=trapz(t,ec);
fprintf('\tISE=%.2i \n',ISE);


% Procesamiento SOPDT
figure('Name','SOPDT');
if(cte==1)
    FT2=fittype("Kp*(1-exp(-(x-L)/T).*(1+(x-L)/T)).*(x>L)");
    SOPDT=fit(t',tension',FT2,'StartPoint',[0.01 0.01 0.01]);
elseif (cte==2)
    FT2=fittype("Kp*(1+(1/T1)/((1/T2)-(1/T1))*exp(-(x-L)/T2)-(1/T2)/((1/T2)-(1/T1))*exp(-(x-L)/T1)).*(x>L)");
    opts = fitoptions( 'Method', 'NonlinearLeastSquares' );
    opts.Display = 'Off';
    opts.Lower = [0 0 0 0];
    opts.StartPoint = [0.913375856139019 0.278498218867048 0.743132468124916 0.392227019534168];
    SOPDT=fit(t',tension',FT2,opts);
else 
    error("Valor invalido.");
end
plot(SOPDT,t',tension');grid on; grid minor;
title('SOPDT model');
xlabel('Tiempo[seg]','Interpreter','Latex');
ylabel('V(t)[V]','Interpreter','Latex');
legend('Muestra','Aproximación','Location','SouthEast');
parametros=coeffvalues(SOPDT);
Kp=parametros(1);
fprintf("\n \nLista de parámetros para la aproximación SOPDT:\n");
if(cte==1)
    L=parametros(2);
    T=parametros(3);
    fprintf('\tKp=%.2i \n',Kp);
    fprintf('\tT=%.2i \n',T);
    fprintf('\tL=%.2i \n',L);
    fprintf('y(t)=%.2i*(1-exp(-(t-%.2i)/%.2i)*(1+(t-%.2i)/%.2i))*u(t-%.2i)\n',Kp,L,T,L,T,L);
    
    Sal_SOPDT=SOPDT(t)';
ec=(tension-Sal_SOPDT).^2;
fprintf("\nIndice de desempeño SOPDT:\n");
ISE=trapz(t,ec);
fprintf('\tISE=%.2i \n',ISE);

elseif(cte==2)
    L=parametros(2);
    T1=parametros(3);
    T2=parametros(4);
    fprintf('\tKp=%.2i \n',Kp);
    fprintf('\tT1=%.2i \n',T1);
    fprintf('\tT2=%.2i \n',T2);
    fprintf('\tL=%.2i \n',L);
    fprintf("y(t)=Kp*(1+(1/T1)/((1/T2)-(1/T1))*exp(-(t-L)/T2)-(1/T2)/((1/T2)-(1/T1))*exp(-(t-L)/T1))*u(t-L)");
    
    Sal_SOPDT=SOPDT(t)';
ec=(tension-Sal_SOPDT).^2;
fprintf("\n\nIndice de desempeño SOPDT:\n");
ISE=trapz(t,ec);
fprintf('\tISE=%.2i \n',ISE);
end


% Procesamiento para TO
figure('Name','TO');
if(cte==1)
    FT3=fittype("Kp*(1-exp(-x/T)*(1+x/T+x^2/(2*T^2))).*(x>0)");
    opts = fitoptions( 'Method', 'NonlinearLeastSquares' );
    opts.Display = 'Off';
    opts.StartPoint = [0.251083857976031 0.616044676146639];
    TO=fit(t',tension',FT3,opts);
elseif (cte==2)
    FT3=fittype("Kp*(1-(T1^2)/((T1-T2)*(T1-T3))*exp(-x/T1)+(T2^2)/((T1-T2)*(T2-T3))*exp(-x/T2)-(T3^2)/((T1-T3)*(T2-T3))*exp(-x/T3)).*(x>0)");
    opts = fitoptions( 'Method', 'NonlinearLeastSquares' );
    opts.Display = 'Off';
    opts.Lower = [0 0 0 0];
    opts.StartPoint = [0.118997681558377 0.959743958516081 0.340385726666133 0.814284826068816];
    TO=fit(t',tension',FT3,opts);
else 
    error("Valor invalido.");
end
plot(TO,t',tension');grid on; grid minor;
title('TO model');
xlabel('Tiempo[seg]','Interpreter','Latex');
ylabel('V(t)[V]','Interpreter','Latex');
legend('Muestra','Aproximación','Location','SouthEast');
parametros=coeffvalues(TO);
fprintf("\n \nLista de parámetros para la aproximación TO:\n");
Kp=parametros(1);
fprintf('\tKp=%.2i \n',Kp);
if(cte==1)
    T=parametros(2);
    fprintf('\tT=%.2i \n',T);
    fprintf('y(t)=%.2i*(1-exp(-t/%.2i)*(1+t/%.2i+t^2/%.2i)).*u(t) \n',...
    Kp,T,T,2*T^2);

Sal_TO=TO(t)';
ec=(tension-Sal_TO).^2;
fprintf("\nIndice de desempeño TO:\n");
ISE=trapz(t,ec);
fprintf('\tISE=%.2i \n',ISE);
    
elseif (cte==2)
    T1=parametros(2);
    T2=parametros(3);
    T3=parametros(4);
    fprintf('\tT1=%.2i \n',T1);
    fprintf('\tT2=%.2i \n',T2);
    fprintf('\tT3=%.2i \n',T3);
    fprintf('y(t)=%.2i*(1-%.2i^2*exp(-t/%.2i)+%.2i*exp(-t/%.2i)-%.2i*exp(-t/%.2i))*u(t) \n',...
    Kp,T1/(T1-T2)*(T1-T3),T1,T2^2/((T1-T2)*(T2-T3)),T2,T3^2/((T1-T3)*(T2-T3)),T3);

Sal_TO=TO(t)';
ec=(tension-Sal_TO).^2;
fprintf("\nIndice de desempeño TO:\n");
ISE=trapz(t,ec);
fprintf('\tISE=%.2i \n',ISE);
end

