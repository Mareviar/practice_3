from django.shortcuts import render, redirect
from django.http import JsonResponse, HttpResponse
import socket
import json

def redirect_to_create_link(request):
    return redirect('create_link/')

def create_link(request):
    if request.method == 'POST':
        try:
            input_value = request.POST['inputValue']
        except KeyError:
            return JsonResponse({'error': 'Bad Request: Missing inputValue field'}, status=400)

        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect(('10.241.125.222', 6379))
        request_data = f'post\n{input_value}\n'
        client.send(request_data.encode('utf-8'))
        response_data = client.recv(1024).decode('utf-8')
        client.close()
        return HttpResponse(response_data)
    else:
        return render(request, 'main/index.html')


def get_link(request, value):
    if value == 'favicon.ico':
        return HttpResponse(status=204)
    elif value == 'create_link':
        return render(request, 'myapp/index.html')
    else:
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect(('10.241.125.222', 6379))
        request_data = f'get\nhttp://10.241.125.222/{value}\n'
        client.send(request_data.encode('utf-8'))
        response_data = client.recv(1024).decode('utf-8')
        client.close()
        return redirect(response_data)

    