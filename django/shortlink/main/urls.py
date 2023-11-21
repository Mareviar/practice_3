from django.urls import path
from . import views

urlpatterns = [
    path('', views.redirect_to_create_link),
    path('create_link/', views.create_link, name='create_link'), 
    path('<str:value>/', views.get_link),
]
    